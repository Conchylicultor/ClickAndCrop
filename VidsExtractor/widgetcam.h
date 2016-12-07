#ifndef WIDGETCAM_H
#define WIDGETCAM_H

#include <QWidget>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QPushButton>

#include <opencv2/opencv.hpp>

/**
 * @brief The WidgetCam class
 * Contain the camera view. Is opened in a new windows.
 */
class WidgetCam : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetCam(QWidget *parent = 0);

    /**
     * @brief setCamTitle
     * @param newTitle
     *
     * Set the name of the camera ("1", "2", ...)
     */
    void setCamTitle(const QString &newTitle);

    /**
     * @brief getCamTitle
     * @return the name of camera the field (camTitle)
     */
    QString getCamTitle() const;

    /**
     * @brief setFinalSize
     * @param newSize
     *
     * Function to receive the size of the capture frame from the main window
     */
    void setFinalSize(const cv::Size &newSize);

signals:
    /**
     * @brief capture
     *
     * Signal send to the main window when the user extract something from the video
     */
    void capture(cv::Mat&);

public slots:

private slots:
    /**
     * @brief openVid
     *
     * Open the video file (open a file dialog)
     */
    void openVid();

    /**
     * @brief updateVideo
     * Function called each 20ms by the timer. It compute the next frame to plot, update the slider and the time.
     */
    void updateVideo();

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
     * @brief slowDown
     * Slow down the video
     */
    void slowDown();

    /**
     * @brief sliderValueChanged
     * @param val
     *
     * Called when the user change the value of the slider.
     * This function update the video.
     */
    void sliderValueChanged(int val);

protected:
    /**
     * @brief closeEvent
     * @param event
     *
     * Reimplementation of the closeEvent function from QMainWindow
     * This function release the video and stop the timer
     */
    void closeEvent(QCloseEvent *event);

    /**
     * @brief eventFilter
     * @param object
     * @param event
     *
     * Reimplemented from the QWidget function. It intercept the key and mouse event from the childs widget in order to handel the pause and so one.
     */
    bool eventFilter(QObject *object, QEvent *event);

private:
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

    /**
     * @brief computeSpeed
     * Update the spped of the video (set the interval of the video)
     */
    void computeSpeed();

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
     * Coordonates of the rectangle
     */
    cv::Point ptTopLeft;
    cv::Point ptBottomRight;

    /**
     * @brief colorFrame
     * Color of the rectangle
     */
    cv::Scalar colorFrame;

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
     * @brief speedFactor
     * Indicate the speed of the video (>0 mean faster than x1, <0 mean slower than x1).
     * After modify this variable, we have to call computeSpeed(); to really change the speed of the video.
     */
    int speedFactor;

    /**
     * Those QWidget represent the differents elements of the interface
     */
    QLineEdit *camTitle;// Name of the camera ("1", "2",... by defaut)
    QLabel *videoLabel;// Frame where the video will be plot
    QPushButton *buttonPlay;// Play/pause button
    QSlider *sliderVid;// Slider to change the current time of the video
    QLabel *currentTimeLabel;// Show the remaining time of the video (ex: 1:23:45/1:59:00)

};

#endif // WIDGETCAM_H
