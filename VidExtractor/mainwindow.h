#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QUrl>
#include <QSpinBox>
#include <QPushButton>
#include <QSlider>

#include <opencv2/opencv.hpp>

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

private:
    /**
     * @brief capturedVid
     * Video sequence container
     */
    cv::VideoCapture capturedVid;

    /**
     * @brief capturedVid
     * Current frame of the video
     */
    cv::Mat currentFrame;
    /**
     * @brief displayedFrame
     * Frame which is plot (with the rectangle)
     */
    cv::Mat displayedFrame;
    /**
     * Contain the last extracted image, which will be eventually saved.
     */
    cv::Mat finalExtractedImage;

    /**
     * @brief colorFrame
     * Color of the rectangle
     */
    cv::Scalar colorFrame;

    /**
     * Coordonates of the rectangle
     */
    cv::Point ptTopLeft;
    cv::Point ptBottomRight;

    /**
     * @brief posCursor
     * Current position of the cursor()
     */
    cv::Point posCursor;
    /**
     * @brief finalSize
     * Size in pixel of the extracted picture
     */
    cv::Size finalSize;
    /**
     * @brief gamma
     * Factor of zoom of the size of the rectangle compare to the final size
     */
    float gamma;

    /**
     * @brief videoLabel
     * Frame where the video will be plot
     */
    QLabel *videoLabel;
    /**
     * @brief extractedImageLabel
     * Frame where the extracted picture will be plot
     */
    QLabel *extractedImageLabel;
    /**
     * @brief gammaScaleVid
     * If the video is too small, it is resize by this scale factor
     */
    float gammaScaleVid;

    /**
     * @brief destinationFolderLabel
     * Show the directory where the pictures will be saved
     */
    QLabel *destinationFolderLabel;
    /**
     * @brief destinationFolder
     * Folder where the pictures will be saved
     */
    QUrl destinationFolder;

    /**
     * @brief checkBoxIsAutoSave
     * If checked, the extracted pictures are automatically saved
     */
    QCheckBox *checkBoxIsAutoSave;

    /**
     * @brief timerPlay
     * Timer to update the video when playing
     */
    QTimer *timerPlay;
    /**
     * @brief isVideo
     * Equal true if we have loaded a video yet. Equal false otherwise
     */
    bool isVideo;

    /**
     * @brief buttonPlay
     * Play/pause button
     */
    QPushButton *buttonPlay;
    /**
     * @brief sliderVid
     * Slider to change the current time of the video
     */
    QSlider *sliderVid;
    /**
     * @brief currentTimeLabel
     * Show the remaining time of the video (ex: 1:23:45/1:59:00)
     */
    QLabel *currentTimeLabel;

    /**
     * @brief imputHeight
     * To choose the height in pixel of the extracted picture
     */
    QSpinBox *imputHeight;
    /**
     * @brief imputWidth
     * To choose the width in pixel of the extracted picture
     */
    QSpinBox *imputWidth;
    /**
     * @brief imputSaveName
     * File name from the picture which will saved
     */
    QLineEdit *imputSaveName;
    /**
     * @brief imputCounter
     * Counter of pictures captured
     */
    QSpinBox *imputCounter;
    /**
     * @brief hasFocus
     * Equal zero if the main window has grab the keyboard. >0 if an imput field has the focus.
     */
    int hasFocus; // Disable the global shortcut space

    /**
     * @brief speedFactor
     * Indicate the speed of the video (>0 mean faster than x1, <0 mean slower than x1).
     * After modify this variable, we have to call computeSpeed(); to really change the speed of the video.
     */
    int speedFactor;
    /**
     * @brief computeSpeed
     * Update the spped of the video (set the interval of the video)
     */
    void computeSpeed();
    /**
     * @brief refreshFrame
     * Plot the currentFrame (with the capture rectangle at the mouse position)
     */
    void refreshFrame();
    /**
     * @brief refreshFrame
     * Plot the currentFrame (with the capture rectangle at the mouse position)
     */
    void updateLabelVideoTime();

private slots:
    /**
     * @brief handleChooseFile
     *
     * Open the video file (open a file dialog)
     */
    void handleChooseFile();
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
     * @brief handlePause
     * Play or pause the video.
     */
    void handlePause();
    /**
     * @brief speedUp
     * Speed up the video
     */
    void speedUp();
    /**
     * @brief speedDown
     * Slow down the video
     */
    void speedDown();
    /**
     * @brief updateVideo
     * Function called each 20ms by the timer. It compute the next frame to plot, update the slider and the time.
     */
    void updateVideo();
    /**
     * @brief updateHeight
     * @param val
     * Change the size of the capture rectangle
     */
    void updateHeight(int val);
    /**
     * @brief updateWidth
     * @param val
     * Change the size of the capture rectangle
     */
    void updateWidth(int val);
    /**
     * @brief sliderValueChanged
     * @param val
     *
     * Called when the user change the value of the slider.
     * This function update the video.
     */
    void sliderValueChanged(int val);
    /**
     * @brief save
     *
     * Save the current finalExtractedImage to the directory specify in destinationFolder.
     * Some computation are made to compute the save name according to the field imputSaveName
     */
    void save();

protected:
    /**
     * @brief event
     * @param event
     * @return
     *
     * Reimplemented from the parent function. Handle the keys event.
     */
    bool event(QEvent *event);
    /**
     * @brief sliderValueChanged
     * @param val
     *
     * Reimplemented from the QWidget function. It intercept the key and mouse event from the childs widget in order to handel the pause and so one.
     */
    bool eventFilter(QObject *object, QEvent *event);

};

#endif // MAINWINDOW_H
