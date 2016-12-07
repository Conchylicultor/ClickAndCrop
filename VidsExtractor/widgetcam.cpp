#include "widgetcam.h"

#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QTime>
#include <QtMath>

#include <QFileDialog>
#include <QMessageBox>



WidgetCam::WidgetCam(QWidget *parent) :
    QWidget(parent), colorFrame(37,153,210)
{
    // ---------- Define Widgets ----------

    QLabel *camLabel = new QLabel("Cam : ");
    camLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    camLabel->installEventFilter(this);

    camTitle = new QLineEdit;
    camTitle->installEventFilter(this);

    QPushButton *buttonBrowse = new QPushButton("Browse...");
    buttonBrowse->installEventFilter(this);
    connect(buttonBrowse, SIGNAL(released()), this, SLOT(openVid()));

    videoLabel = new QLabel("Video");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setMouseTracking(true);
    videoLabel->installEventFilter(this);
    videoLabel->setContentsMargins(0,0,0,0);

    buttonPlay = new QPushButton("Pause");
    buttonPlay->installEventFilter(this);
    connect(buttonPlay, SIGNAL(released()), this, SLOT(handlePause()));

    QPushButton *buttonSpeedUp = new QPushButton(">");
    buttonSpeedUp->installEventFilter(this);
    connect(buttonSpeedUp, SIGNAL(released()), this, SLOT(speedUp()));
    QPushButton *buttonSpeedDown = new QPushButton("<");
    buttonSpeedDown->installEventFilter(this);
    connect(buttonSpeedDown, SIGNAL(released()), this, SLOT(slowDown()));

    sliderVid = new QSlider(Qt::Horizontal);
    sliderVid->setMinimum(0);
    sliderVid->setMaximum(100);
    sliderVid->setMinimumWidth(200);
    sliderVid->installEventFilter(this);
    connect(sliderVid, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));

    currentTimeLabel = new QLabel("--/--");
    currentTimeLabel->installEventFilter(this);
    currentTimeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // ---------- Define Layouts ----------

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->addWidget(camLabel);
    titleLayout->addWidget(camTitle);
    titleLayout->addWidget(buttonBrowse);

    QHBoxLayout *navigationLayout = new QHBoxLayout;
    navigationLayout->addWidget(buttonSpeedDown);
    navigationLayout->addWidget(buttonPlay);
    navigationLayout->addWidget(buttonSpeedUp);
    navigationLayout->addWidget(sliderVid);
    navigationLayout->addWidget(currentTimeLabel);

    // Hack to avoid padding
    QHBoxLayout* centerVidH = new QHBoxLayout;
    centerVidH->addStretch();
    centerVidH->addWidget(videoLabel);
    centerVidH->addStretch();

    QVBoxLayout* centerVidV = new QVBoxLayout;
    centerVidV->addStretch();
    centerVidV->addLayout(centerVidH);
    centerVidV->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(titleLayout);
    mainLayout->addLayout(centerVidV);
    mainLayout->addLayout(navigationLayout);

    this->setLayout(mainLayout);

    // ---------- Globals properties ----------

    timerPlay = new QTimer(this);
    connect(timerPlay, SIGNAL(timeout()), this, SLOT(updateVideo()));
    speedFactor = 0;
    computeSpeed();// Set the interval of the timer

    isVideo = false;// Video not loaded yet

    // Gamma factor (zoom of the rectancle)
    gamma = 1.0;
}

void WidgetCam::openVid()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Video"),
                                                    QString(),
                                                    tr("Video Files (*.avi *.mp4)"));

    // If not cancel
    if(!fileName.isNull())
    {
        this->setWindowTitle(fileName);

        capturedVid.open(fileName.toStdString());

        if(!capturedVid.isOpened())
        {
            QMessageBox::critical(this,
                                  "Incorrect File",
                                  "The video can not be open.");
        }
        else
        {
            isVideo = true;
            timerPlay->start();
        }
    }
}

void WidgetCam::updateVideo()
{
    // We read the next frame
    if (!capturedVid.read(currentFrame))
    {
        timerPlay->stop();
        isVideo = false;
    }

    // Actualisation of the slider
    //float currentPosition = capturedVid.get(CV_CAP_PROP_POS_AVI_RATIO);// Bug with ffmpeg
    float currentPosition = capturedVid.get(CV_CAP_PROP_POS_FRAMES)/capturedVid.get(CV_CAP_PROP_FRAME_COUNT);

    bool previousState = sliderVid->blockSignals(true);
    sliderVid->setValue(currentPosition*100);
    sliderVid->blockSignals(previousState);

    // Acturalisation of the label
    updateLabelVideoTime();

    refreshFrame();
}

void WidgetCam::refreshFrame()
{

    //displayedFrame = currentFrame.clone();
    cv::cvtColor(currentFrame, displayedFrame, CV_BGR2RGB);// Convertion in the right color space

    // Plot the rectangle
    ptTopLeft.x = posCursor.x - gamma * finalSize.width / 2;
    ptTopLeft.y = posCursor.y - gamma * finalSize.height / 2;

    ptBottomRight.x = posCursor.x + gamma * finalSize.width / 2;
    ptBottomRight.y = posCursor.y + gamma * finalSize.height / 2;

    cv::rectangle(displayedFrame, ptTopLeft, ptBottomRight, colorFrame);

    // Convert into QImage
    QImage imagePlot (displayedFrame.data,
                      displayedFrame.cols,
                      displayedFrame.rows,
                      displayedFrame.step,
                      QImage::Format_RGB888);

    videoLabel->setPixmap(QPixmap::fromImage(imagePlot));
}

void WidgetCam::updateLabelVideoTime()
{
    if(isVideo)
    {
        // Recuperation of the current time and the ratio time
        float currentPosMsec = capturedVid.get(CV_CAP_PROP_POS_MSEC);
        //float currentPosRatio = capturedVid.get(CV_CAP_PROP_POS_AVI_RATIO); // Bug with ffmpeg
        float currentPosFrame = capturedVid.get(CV_CAP_PROP_POS_FRAMES);
        float currentPosFrameCount = capturedVid.get(CV_CAP_PROP_FRAME_COUNT);

        // Convertion in Qt format
        QTime currentTime(0,0);
        currentTime = currentTime.addMSecs(currentPosMsec);
        QTime totalTime(0,0);
        //totalTime = totalTime.addMSecs(currentPosMsec/currentPosRatio); // Bug with ffmpeg
        totalTime = totalTime.addMSecs(currentPosMsec*currentPosFrameCount/currentPosFrame);

        // Plot
        currentTimeLabel->setText(currentTime.toString() + "/" + totalTime.toString());
    }
}

void WidgetCam::handlePause()
{
    if(isVideo)
    {
        if(timerPlay->isActive())
        {
            timerPlay->stop();
            buttonPlay->setText("Play");
        }
        else
        {
            timerPlay->start();
            buttonPlay->setText("Pause");
        }
    }
}

void WidgetCam::computeSpeed()
{
    // Expodential function (f(0)=25ms)
    int newInterval = 25*(qPow(1.6,speedFactor));
    timerPlay->setInterval(newInterval);
    // Show the wrong value !! (just an indicator)
    buttonPlay->setToolTip("Speed : x" + QString::number(25.0/newInterval, 'g', 2));
}

void WidgetCam::speedUp()
{
    speedFactor--;// Increase the speed by decreasing the interval
    computeSpeed();
}

void WidgetCam::slowDown()
{
    speedFactor++;// Decrease the speed by increasing the interval
    computeSpeed();
}

void WidgetCam::sliderValueChanged(int val)
{
    if(isVideo)
    {
        // Set the time
        //capturedVid.set(CV_CAP_PROP_POS_AVI_RATIO, val/100.0);// Bug with ffmpeg
        capturedVid.set(CV_CAP_PROP_POS_FRAMES, val/100.0*capturedVid.get(CV_CAP_PROP_FRAME_COUNT));

        // Read the new current frame
        if (!capturedVid.read(currentFrame))
        {
            timerPlay->stop();
            isVideo = false;
        }

        updateLabelVideoTime();
        refreshFrame();
    }
}

void WidgetCam::setCamTitle(const QString &newTitle)
{
    camTitle->setText(newTitle);
}

QString WidgetCam::getCamTitle() const
{
    return camTitle->text();
}

void WidgetCam::setFinalSize(const cv::Size &newSize)
{
    finalSize = newSize;
}

void WidgetCam::closeEvent(QCloseEvent *event)
{
    timerPlay->stop();
    capturedVid.release();
}

bool WidgetCam::eventFilter(QObject *object, QEvent *event)
{
    // Capture the key pressed
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        int playJump = 5000;

        // Pause
        if(keyEvent->key() == Qt::Key_Space)
        {
            handlePause();
            return true;
        }
        // Move backward in the video
        else if(keyEvent->key() == Qt::Key_Left && keyEvent->modifiers() & Qt::ControlModifier)
        {
            capturedVid.set(CV_CAP_PROP_POS_MSEC, capturedVid.get(CV_CAP_PROP_POS_MSEC) - playJump);
            updateVideo();
            return true;
        }
        // Move forward in the video
        else if(keyEvent->key() == Qt::Key_Right && keyEvent->modifiers() & Qt::ControlModifier)
        {
            capturedVid.set(CV_CAP_PROP_POS_MSEC, capturedVid.get(CV_CAP_PROP_POS_MSEC) + playJump);
            updateVideo();
            return true;
        }
    }
    else if (object == videoLabel && isVideo)
    {
        if(event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            posCursor.x = mouseEvent->x();
            posCursor.y = mouseEvent->y();

            // If the video is in pause (or has a long refresh delay), we refresh the frame
            if(!timerPlay->isActive() || timerPlay->interval() > 30)
            {
                refreshFrame();
            }

            // We give the focus to the window (to grap the keys events)
            this->activateWindow();

            return true;
        }
        if(event->type() == QEvent::Wheel)
        {
            QWheelEvent *mouseWheelEvent = static_cast<QWheelEvent *>(event);

            float factor = 0.001f;
            if(mouseWheelEvent->modifiers() & Qt::ControlModifier)// Soft modifier
            {
                factor = 0.0001f;
            }

            gamma += factor * mouseWheelEvent->delta();

            if(gamma < 0.0)
            {
                gamma = -gamma;
            }

            // If the video is in pause, we refresh the frame
            if(!timerPlay->isActive() || timerPlay->interval() > 30)
            {
                refreshFrame();
            }

            return true;
        }
        if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if(mouseEvent->button() == Qt::LeftButton)
            {
                // Extraction of the capture
                cv::Mat croppedImage;

                try {
                    croppedImage = currentFrame(cv::Rect(ptTopLeft, ptBottomRight));
                }
                catch (...)
                {

                    QMessageBox::critical(this,
                                          "Extraction failed",
                                          "Do not extract outside the frame");
                    return true;
                }

                // We send the extracted image to the main windows
                emit capture(croppedImage);
            }

            return true;
        }
    }

    return false;
}

