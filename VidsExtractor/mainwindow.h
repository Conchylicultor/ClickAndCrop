#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "widgetcam.h"

#include <QMainWindow>
#include <QSpinBox>
#include <QCheckBox>

#include <QList>
#include <QMap>
#include <QUrl>

#include <opencv2/opencv.hpp>

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief MainWindow
     * @param parent
     *
     * Constructor of the main window
     */
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    /**
     * @brief closeEvent
     * @param event
     *
     * Reimplementation of the closeEvent function from QMainWindow
     * This function close all the opened windows (camera views)
     */
    void closeEvent(QCloseEvent *event);

private slots:
    /**
     * @brief addCam
     *
     * When clicking on the "+" button.
     * Open a camera view (create a windows contain WidgetCam)
     */
    void addCam();

    /**
     * @brief handleChooseDestination
     *
     * Open a file dialogue to select the directory where the pictures will be saved
     */
    void handleChooseDestination();

    /**
     * @brief handleOpenDestination
     *
     * Open the directory in the explorer
     */
    void handleOpenDestination();

    /**
     * @brief sizeChanged
     *
     * Slot called when we modified the final size of the picture which will be saved (with the spinBox height and width).
     * It modify the size of the capture zone in all the camera views
     */
    void sizeChanged();

    /**
     * @brief receivedCapture
     * @param newCapture
     *
     * Slot called when a picture is extracted from a video.
     * It refresh the extractedImageLabel with the image contain in newCapture, send from a WidgetCam.
     */
    void receivedCapture(cv::Mat &newCapture);

    /**
     * @brief save
     *
     * Save the current finalExtractedImage to the directory specify in destinationFolder.
     * Some computation are made to compute the save name according to the field imputSaveName
     */
    void save();

private:
    /**
     * @brief replaceCountString
     *
     * This function replace the character "character" in the string "str" by the integer "value".
     * For instance replaceCountString("img#.jpg", '#', 13) will return "img13.jpg"
     * If there is more than one character character in the string, it will be completed by some zero.
     * For instance replaceCountString("img####.jpg", '#', 13) will return "img0013.jpg"
     *
     * @param str
     * @param character
     * @param value
     * @return the replaced string (see description)
     */
    QString replaceCountString(const QString &str, QChar character, int value);

    /**
     * @brief listWidgetCam
     * List of the camera views (windows which contain a camera)
     */
    QList<WidgetCam*> listWidgetCam;

    /**
     * @brief lastWidgetCam
     * Contain the name of the last camera which has capture something (for the save name)
     */
    QString lastWidgetCam;

    /**
     * @brief destinationFolder
     * Folder where the pictures will be saved
     */
    QUrl destinationFolder;

    /**
     * @brief mapPersons
     * 2 dimentional map which contain the number of time that person number idOfCurrentPerson has been
     * extracted by camera "nameOfCam"
     * Use: int nbOfOccurence = mapPersons["nameOfCam"][idOfCurrentPerson];
     * Used to compute the save name.
     */
    QMap<QString, QMap<int, int> > mapPersons;

    /**
     * Those QWidget represent the differents elements of the interface
     */
    QPushButton *buttonChooseDestination;
    QLabel *destinationFolderLabel;
    QLineEdit *imputSaveName;
    QSpinBox *imputCurrentPerson;
    QSpinBox *imputHeight;
    QSpinBox *imputWidth;
    QCheckBox *checkBoxIsAutoSave;
    QLabel *extractedImageLabel;
    QLabel *extractedImageName;

    /**
     * Contain the last extracted image, which will be eventually saved.
     */
    cv::Mat finalExtractedImage;
};

#endif // MAINWINDOW_H
