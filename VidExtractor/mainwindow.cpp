#include "mainwindow.h"

#include <QtGui>
#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), colorFrame(37,153,210), gamma(1.0), gammaScaleVid(1.0)
{
    // ---------- Create widgets ----------

    // Controls buttons (load/save dialog)
    QPushButton *buttonChooseVideoFile = new QPushButton("Choose source file");
    connect(buttonChooseVideoFile, SIGNAL(released()),this, SLOT(handleChooseFile()));

    QPushButton *buttonChooseDestination = new QPushButton("Choose destination");
    connect(buttonChooseDestination, SIGNAL(released()),this, SLOT(handleChooseDestination()));

    destinationFolderLabel = new QLabel("No directory ...");
    destinationFolderLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QPushButton *buttonOpenDestination = new QPushButton("Open...");
    connect(buttonOpenDestination, SIGNAL(released()),this, SLOT(handleOpenDestination()));

    // Image frames
    videoLabel = new QLabel("Video");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setMouseTracking(true);
    videoLabel->installEventFilter(this);

    videoLabel->setContentsMargins(0,0,0,0);

    extractedImageLabel = new QLabel("Extracted Image");
    extractedImageLabel->setAlignment(Qt::AlignCenter);

    // Decoration
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    // Destination size buttons
    imputHeight = new QSpinBox;
    imputHeight->installEventFilter(this);
    connect(imputHeight, SIGNAL(valueChanged(int)),this, SLOT(updateHeight(int)));
    imputHeight->setMaximum(500);
    imputHeight->setValue(128);

    imputWidth = new QSpinBox;
    imputWidth->installEventFilter(this);
    connect(imputWidth, SIGNAL(valueChanged(int)),this, SLOT(updateWidth(int)));
    imputWidth->setMaximum(500);
    imputWidth->setValue(48);

    // Save buttons
    checkBoxIsAutoSave = new QCheckBox("Auto-save");
    checkBoxIsAutoSave->setCheckState(Qt::Checked);
    imputSaveName = new QLineEdit("img####.bmp");
    imputSaveName->installEventFilter(this);
    QPushButton *buttonSave = new QPushButton("Save");
    connect(buttonSave, SIGNAL(released()), this, SLOT(save()));
    imputCounter = new QSpinBox;
    imputCounter->installEventFilter(this);
    imputCounter->setMaximum(9999);

    // Navigation widget
    buttonPlay = new QPushButton("Pause");
    connect(buttonPlay, SIGNAL(released()), this, SLOT(handlePause()));

    QPushButton *buttonSpeedUp = new QPushButton(">");
    connect(buttonSpeedUp, SIGNAL(released()), this, SLOT(speedUp()));
    QPushButton *buttonSpeedDown = new QPushButton("<");
    connect(buttonSpeedDown, SIGNAL(released()), this, SLOT(speedDown()));

    sliderVid = new QSlider(Qt::Horizontal);
    sliderVid->setMinimum(0);
    sliderVid->setMaximum(100);
    sliderVid->setMinimumWidth(200);
    connect(sliderVid, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));

    currentTimeLabel = new QLabel("--/--");
    currentTimeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // ---------- Set layout ----------
    QGridLayout *layoutMain = new QGridLayout;
    QHBoxLayout *layoutPlaybar = new QHBoxLayout;
    QVBoxLayout *layoutControls = new QVBoxLayout;

    QHBoxLayout *layoutControlsRow1 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow1_5 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow2 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow3 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow4 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow5 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow6 = new QHBoxLayout;

    // Hack to avoid padding
    QHBoxLayout* centerVidH = new QHBoxLayout;
    centerVidH->addStretch();
    centerVidH->addWidget(videoLabel);
    centerVidH->addStretch();

    QVBoxLayout* centerVidV = new QVBoxLayout;
    centerVidV->addStretch();
    centerVidV->addLayout(centerVidH);
    centerVidV->addStretch();

    layoutMain->addLayout(centerVidV,0,0);
    layoutMain->addLayout(layoutPlaybar,1,0);
    layoutMain->addLayout(layoutControls,0,1,-1,1);

    layoutPlaybar->addWidget(buttonSpeedDown);
    layoutPlaybar->addWidget(buttonPlay);
    layoutPlaybar->addWidget(buttonSpeedUp);
    layoutPlaybar->addWidget(sliderVid);
    layoutPlaybar->addWidget(currentTimeLabel);

    layoutControlsRow1->addWidget(buttonChooseVideoFile);
    layoutControlsRow1_5->addWidget(buttonChooseDestination);
    layoutControlsRow2->addWidget(destinationFolderLabel);
    layoutControlsRow2->addWidget(buttonOpenDestination);
    layoutControlsRow3->addWidget(new QLabel("Height :"));
    layoutControlsRow3->addWidget(imputHeight);
    layoutControlsRow3->addWidget(new QLabel("Width :"));
    layoutControlsRow3->addWidget(imputWidth);
    layoutControlsRow4->addWidget(checkBoxIsAutoSave);
    layoutControlsRow5->addWidget(imputSaveName);
    layoutControlsRow5->addWidget(buttonSave);
    layoutControlsRow6->addWidget(new QLabel("Counter :"));
    layoutControlsRow6->addWidget(imputCounter);

    layoutControls->addLayout(layoutControlsRow1);
    layoutControls->addLayout(layoutControlsRow1_5);
    layoutControls->addLayout(layoutControlsRow2);
    layoutControls->addLayout(layoutControlsRow3);
    layoutControls->addLayout(layoutControlsRow4);
    layoutControls->addLayout(layoutControlsRow5);
    layoutControls->addLayout(layoutControlsRow6);
    layoutControls->addWidget(separator);
    layoutControls->addWidget(extractedImageLabel);

    // Set layout in QWidget
    QWidget *window = new QWidget();
    window->setLayout(layoutMain);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);


    // ---------- Global properties ----------
    this->setWindowTitle("VidExtractor");
    this->grabKeyboard(); // Grab the sapce shortcut to control the video navigation
    hasFocus = 0;

    timerPlay = new QTimer(this);
    speedFactor = 0;
    computeSpeed();// Set the interval of the timer
    connect(timerPlay, SIGNAL(timeout()), this, SLOT(updateVideo()));

    isVideo = false;// Video not loaded yet

}

void MainWindow::handleChooseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Video"),
                                                    QString(),
                                                    tr("Video Files (*.avi *.mp4)"));

    // If not cancel
    if(!fileName.isNull())
    {
        this->setWindowTitle("VidExtractor - " + fileName);

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

void MainWindow::handlePause()
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

void MainWindow::computeSpeed()
{
    // Expodential function (f(0)=25ms)
    int newInterval = 25*(qPow(1.6,speedFactor));
    timerPlay->setInterval(newInterval);
    // Show the wrong value !! (just an indicator)
    buttonPlay->setToolTip("Speed : x" + QString::number(25.0/newInterval, 'g', 2));
}

void MainWindow::speedUp()
{
    speedFactor--;// Increase the speed by decreasing the interval
    computeSpeed();
}

void MainWindow::speedDown()
{
    speedFactor++;// Decrease the speed by increasing the interval
    computeSpeed();
}

void MainWindow::handleChooseDestination()
{
    QUrl directoryName = QFileDialog::getExistingDirectoryUrl(this,
                                                              tr("Select destination"));

    if(directoryName.isEmpty()) // Cancel button
    {
        return;
    }
    else if(directoryName.isValid())
    {
        destinationFolder = directoryName;
        // Crop the string if too long
        QFontMetricsF *fontElide = new QFontMetricsF(QFont());
        QString elidedDestination = fontElide->elidedText("Destination : " + directoryName.toString(), Qt::ElideMiddle, 400);
        delete fontElide;
        // Plot the string
        destinationFolderLabel->setText(elidedDestination);
    }
    else
    {
        QMessageBox::critical(this,
                              "Incorrect directory",
                              "Incorrect directory. Please choose an other one.");
    }
}

void MainWindow::handleOpenDestination()
{
    QDesktopServices::openUrl(destinationFolder);
}

void MainWindow::updateVideo()
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

void MainWindow::updateHeight(int val)
{
    finalSize.height = val;
}

void MainWindow::updateWidth(int val)
{
    finalSize.width = val;
}

void MainWindow::sliderValueChanged(int val)
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

void MainWindow::save()
{
    // Check if there is an image to save
    if(finalExtractedImage.empty())
    {
        QMessageBox::critical(this,
                              "No image to save",
                              "First try to extract an image");
        return;
    }

    // Check if the directory is valid
    if(!destinationFolder.isValid())
    {

        QMessageBox::critical(this,
                              "Incorrect directory",
                              "Impossible to save in the current directory");
        return;
    }

    // Compute the filename
    int lengthCounter = imputSaveName->text().count('#');
    QString decimalPart = QString::number(imputCounter->value());
    if(lengthCounter > 1)
    {
        decimalPart = decimalPart.rightJustified(lengthCounter, '0', true);
    }

    QString filename = imputSaveName->text();

    if(lengthCounter > 0)
    {
        QString sharps;
        sharps = sharps.rightJustified(lengthCounter, '#');
        filename.replace(sharps, decimalPart);

        //filename.replace("#+", decimalPart);
    }

    // Save the picture
    try
    {
        bool success = cv::imwrite(destinationFolder.toLocalFile().toStdString() + "/" + filename.toStdString(), finalExtractedImage);
        if(!success)
        {
            QMessageBox::critical(this,
                                  "Save failed",
                                  "Error while saving the image. Please check the file name.");
            return;
        }
    }
    catch(...)
    {
        QMessageBox::critical(this,
                              "Save failed",
                              "Error while saving the image. Please check the file format.");

        std::cout << destinationFolder.toLocalFile().toStdString() + "/" + filename.toStdString() << std::endl;

        return;
    }

    // Increment the counter if necessary
    if(lengthCounter > 0)
    {
        imputCounter->setValue(imputCounter->value() + 1);
    }
}

void MainWindow::refreshFrame()
{

    // Convertion in the right color space
    //displayedFrame = currentFrame.clone();
    cv::cvtColor(currentFrame, displayedFrame, CV_BGR2RGB);

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

    // Resize the vid but pb whith mouse position
    float previousWidth = imagePlot.width();
    float nextWidth = previousWidth;

    if(imagePlot.width() < 600 || imagePlot.height() < 350)
    {
        imagePlot = imagePlot.scaled(QSize(600,350), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        nextWidth = imagePlot.width();
    }
    gammaScaleVid = nextWidth/previousWidth;
    //std::cout << gammaScaleVid << std::endl;

    videoLabel->setPixmap(QPixmap::fromImage(imagePlot));
}

void MainWindow::updateLabelVideoTime()
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

bool MainWindow::event(QEvent *event)
{
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
    return QMainWindow::event(event);
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == videoLabel && isVideo)
    {
        if(event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            posCursor.x = mouseEvent->x()/gammaScaleVid;
            posCursor.y = mouseEvent->y()/gammaScaleVid;

            // If the video is in pause, we refresh the frame
            if(!timerPlay->isActive() || timerPlay->interval() > 30)
            {
                refreshFrame();
            }

            return true;
        }
        if(event->type() == QEvent::Wheel)
        {
            QWheelEvent *mouseWheelEvent = static_cast<QWheelEvent *>(event);

            float factor = 0.001;
            if(mouseWheelEvent->modifiers() & Qt::ControlModifier)
            {
                factor = 0.0001;
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
                // Extraction of the image
                cv::Mat croppedImage;

                try{
                    croppedImage = currentFrame(cv::Rect(ptTopLeft, ptBottomRight));
                }
                catch (...)
                {

                    QMessageBox::critical(this,
                                          "Extraction failed",
                                          "Do not extract outside the frame");
                    return true;
                }

                cv::resize(croppedImage, finalExtractedImage, finalSize);

                extractedImageLabel->setPixmap(QPixmap::fromImage(
                                                   QImage(finalExtractedImage.data,
                                                          finalExtractedImage.cols,
                                                          finalExtractedImage.rows,
                                                          finalExtractedImage.step,
                                                          QImage::Format_RGB888)));

                // Save image
                if(checkBoxIsAutoSave->isChecked())
                {
                    save();
                }
            }

            return true;
        }
    }
    // Release or grab the keyboard if we are in an input field
    else if (object == imputHeight ||
             object == imputWidth ||
             object == imputSaveName ||
             object == imputCounter)
    {

        if(event->type() == QEvent::FocusIn)
        {
            hasFocus++;
        }
        else if(event->type() == QEvent::FocusOut)
        {
            hasFocus--;
        }
        else if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            int playJump = 5000;
            if(keyEvent->key() == Qt::Key_Space)
            {
                handlePause();
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Left && keyEvent->modifiers() & Qt::ControlModifier)
            {
                capturedVid.set(CV_CAP_PROP_POS_MSEC, capturedVid.get(CV_CAP_PROP_POS_MSEC) - playJump);
                updateVideo();
                return true;
            }
            else if(keyEvent->key() == Qt::Key_Right && keyEvent->modifiers() & Qt::ControlModifier)
            {
                capturedVid.set(CV_CAP_PROP_POS_MSEC, capturedVid.get(CV_CAP_PROP_POS_MSEC) + playJump);
                updateVideo();
                return true;
            }
        }

        if(hasFocus > 0)
        {
            this->releaseKeyboard();
        }
        else
        {
            this->grabKeyboard();
        }
    }
    return false;
}

MainWindow::~MainWindow()
{
}
