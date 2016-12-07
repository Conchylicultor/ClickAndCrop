#include "mainwindow.h"

#include "widgetcam.h"

#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QSpinBox>

#include <QMessageBox>
#include <QFileDialog>

#include <QDir>
#include <QDesktopServices>

#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ---------- Define Widgets ----------
    QPushButton *buttonAddCam = new QPushButton("+");
    connect(buttonAddCam, SIGNAL(released()), this, SLOT(addCam()));

    QPushButton *buttonChooseDestination = new QPushButton("Choose destination");
    connect(buttonChooseDestination, SIGNAL(released()), this, SLOT(handleChooseDestination()));

    destinationFolderLabel = new QLabel("No directory ...");
    destinationFolderLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QPushButton *buttonOpenDestination = new QPushButton("Open...");
    connect(buttonOpenDestination, SIGNAL(released()),this, SLOT(handleOpenDestination()));

    imputCurrentPerson = new QSpinBox;
    imputCurrentPerson->setMaximum(500);
    imputCurrentPerson->setValue(0);

    // Destination size buttons
    imputHeight = new QSpinBox;
    imputHeight->setMaximum(599);
    imputHeight->setValue(128);

    imputWidth = new QSpinBox;
    imputWidth->setMaximum(599);
    imputWidth->setValue(48);

    connect(imputHeight, SIGNAL(valueChanged(int)), this, SLOT(sizeChanged()));
    connect(imputWidth, SIGNAL(valueChanged(int)), this, SLOT(sizeChanged()));

    // Save buttons
    checkBoxIsAutoSave = new QCheckBox("Auto-save");
    checkBoxIsAutoSave->setCheckState(Qt::Checked);

    imputSaveName = new QLineEdit("cam$_pers%%%%_img####.png");

    QPushButton *buttonSave = new QPushButton("Save");
    connect(buttonSave, SIGNAL(released()), this, SLOT(save()));


    // Decoration
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    // Extracted image
    extractedImageLabel = new QLabel("Extracted Image");
    extractedImageLabel->setAlignment(Qt::AlignCenter);

    extractedImageName = new QLabel();
    extractedImageName->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // ---------- Define Layouts ----------

    QVBoxLayout *layoutMain = new QVBoxLayout;

    QHBoxLayout *layoutControlsRow1 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow2 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow3 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow4 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow5 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow6 = new QHBoxLayout;
    QHBoxLayout *layoutControlsRow7 = new QHBoxLayout;

    layoutControlsRow1->addWidget(new QLabel("Add a cam : "));
    layoutControlsRow1->addWidget(buttonAddCam);

    layoutControlsRow2->addWidget(buttonChooseDestination);
    layoutControlsRow2->addWidget(destinationFolderLabel);
    layoutControlsRow2->addWidget(buttonOpenDestination);

    layoutControlsRow3->addWidget(new QLabel("Person : "));
    layoutControlsRow3->addWidget(imputCurrentPerson);

    layoutControlsRow4->addWidget(new QLabel("Height :"));
    layoutControlsRow4->addWidget(imputHeight);
    layoutControlsRow4->addWidget(new QLabel("Width :"));
    layoutControlsRow4->addWidget(imputWidth);

    layoutControlsRow5->addWidget(checkBoxIsAutoSave);

    layoutControlsRow6->addWidget(imputSaveName);
    layoutControlsRow6->addWidget(buttonSave);

    layoutControlsRow7->addWidget(extractedImageName);

    layoutMain->addLayout(layoutControlsRow1);
    layoutMain->addLayout(layoutControlsRow2);
    layoutMain->addLayout(layoutControlsRow3);
    layoutMain->addLayout(layoutControlsRow4);
    layoutMain->addLayout(layoutControlsRow5);
    layoutMain->addLayout(layoutControlsRow6);
    layoutMain->addWidget(separator);
    layoutMain->addWidget(extractedImageLabel);
    layoutMain->addLayout(layoutControlsRow7);

    // Set layout in QWidget
    QWidget *window = new QWidget();
    window->setLayout(layoutMain);

    // Set QWidget as the central layout of the main window
    setCentralWidget(window);

}

void MainWindow::addCam()
{
    listWidgetCam.append(new WidgetCam);
    listWidgetCam.last()->setCamTitle(QString::number(listWidgetCam.size()));
    listWidgetCam.last()->show();

    connect(listWidgetCam.last(), SIGNAL(capture(cv::Mat&)), this, SLOT(receivedCapture(cv::Mat&)));

    // Update the finalSize of the new window
    sizeChanged();
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

void MainWindow::sizeChanged()
{
    cv::Size finalSize;
    finalSize.height = imputHeight->value();
    finalSize.width = imputWidth->value();
    for (int i = 0; i < listWidgetCam.size(); ++i)
    {
        listWidgetCam.at(i)->setFinalSize(finalSize);
    }
}

void MainWindow::receivedCapture(cv::Mat &newCapture)
{
    cv::Size finalSize;
    finalSize.height = imputHeight->value();
    finalSize.width = imputWidth->value();

    cv::resize(newCapture, finalExtractedImage, finalSize);


    extractedImageLabel->setPixmap(QPixmap::fromImage(
                                       QImage(finalExtractedImage.data,
                                              finalExtractedImage.cols,
                                              finalExtractedImage.rows,
                                              finalExtractedImage.step,
                                              QImage::Format_RGB888)));

    // We save the name of the sender
    lastWidgetCam = qobject_cast<WidgetCam*>(sender())->getCamTitle();

    // Save image
    if(checkBoxIsAutoSave->isChecked())
    {
        save();
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

    // We add the camera and the person to the map if there are not contained yet
    if(!mapPersons.contains(lastWidgetCam))
    {
        mapPersons.insert(lastWidgetCam, QMap<int, int>());
    }

    if(!mapPersons.value(lastWidgetCam).contains(imputCurrentPerson->value()))
    {
        mapPersons[lastWidgetCam].insert(imputCurrentPerson->value(), 0);
    }

    // Computation of the file name
    QString filename = imputSaveName->text();

    bool ok;
    int numCam = lastWidgetCam.toInt(&ok);
    if(ok)
    {
        filename = replaceCountString(filename, '$', numCam);
    }
    else
    {
        filename.replace('$', lastWidgetCam);
    }
    filename = replaceCountString(filename, '%', imputCurrentPerson->value());
    filename = replaceCountString(filename, '#', mapPersons.value(lastWidgetCam).value(imputCurrentPerson->value()));


    // Save the picture
    QFileInfo fileToSave(destinationFolder.toLocalFile() + "/" + filename);
    if(fileToSave.exists())
    {
        int result = QMessageBox::warning(this,
                                          "The file already exist",
                                          "The file already exist, Do you want to replace it ?",
                                          QMessageBox::Save | QMessageBox::Cancel);
        if(result == QMessageBox::Cancel)
        {
            return;
        }
    }

    try
    {
        // We eventualy create the directory
        QDir::root().mkdir(fileToSave.absoluteDir().absolutePath());

        bool success = cv::imwrite(fileToSave.absoluteFilePath().toStdString(), finalExtractedImage);
        if(!success)
        {
            QMessageBox::critical(this,
                                  "Save failed",
                                  "Error while saving the image. Please check the file name.\n" + fileToSave.absoluteFilePath());
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

    extractedImageName->setText (filename);

    // Increment for the next time
    mapPersons[lastWidgetCam][imputCurrentPerson->value()] = mapPersons[lastWidgetCam][imputCurrentPerson->value()] + 1;

}

QString MainWindow::replaceCountString(const QString &str, QChar character, int value)
{
    int lengthCounter = str.count(character);

    QString decimalPart = QString::number(value);
    if(lengthCounter > 1)
    {
        decimalPart = decimalPart.rightJustified(lengthCounter, '0', true);
    }

    QString finalStr = str;

    if(lengthCounter > 0)
    {
        QString sharps;
        sharps = sharps.rightJustified(lengthCounter, character);
        finalStr.replace(sharps, decimalPart);

        //filename.replace("#+", decimalPart);
    }

    return finalStr;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    for (int i = 0; i < listWidgetCam.size(); ++i)
    {
        // Close all windows
        listWidgetCam.at(i)->close();
    }
}

MainWindow::~MainWindow()
{
}
