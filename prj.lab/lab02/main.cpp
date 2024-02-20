#include <opencv2/opencv.hpp>
#include <vector>
#include <random>

// Function to generate the test image with three levels of brightness
cv::Mat generate_test_image(int side, int inner_square_side, int circle_radius, const std::vector<uchar>& levels) {
    cv::Mat image = cv::Mat::zeros(side, side, CV_8UC1);

    // Outer square
    image.setTo(levels[0]);

    // Inner square
    cv::Point inner_square_top_left((side - inner_square_side) / 2, (side - inner_square_side) / 2);
    cv::rectangle(image, inner_square_top_left, inner_square_top_left + cv::Point(inner_square_side, inner_square_side), levels[1], cv::FILLED);

    // Circle
    cv::circle(image, cv::Point(side / 2, side / 2), circle_radius, levels[2], cv::FILLED);

    return image;
}


// Function to draw the histogram of brightness
cv::Mat draw_histogram(const cv::Mat& src) {
    // Calculate the histogram
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    bool uniform = true, accumulate = false;
    cv::Mat hist;
    cv::calcHist(&src, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

    // Draw the histogram
    int hist_w = 256;
    int hist_h = 230;
    cv::Mat histImage(hist_h, hist_w, CV_8UC1, cv::Scalar(230));
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX);

    for(int i = 1; i < histSize; i++) {
        cv::line(histImage, cv::Point(i, hist_h), cv::Point(i, hist_h - cvRound(hist.at<float>(i))), cv::Scalar(0), 1);
    }

    return histImage;
}

// Function to add noise to the image
cv::Mat add_noise(const cv::Mat& src, double stddev) {
    cv::Mat noise = cv::Mat(src.size(), CV_32F);  // Use floating-point precision for noise
    std::normal_distribution<float> dist(0, stddev);
    std::default_random_engine generator;

    // Generate noise
    for (int i = 0; i < noise.rows; i++) {
        for (int j = 0; j < noise.cols; j++) {
            noise.at<float>(i, j) = dist(generator);
        }
    }

    // Add noise to the original image
    cv::Mat noisy_image;
    src.convertTo(noisy_image, CV_32F);  // Convert src to float for addition
    noisy_image += noise;  // Add the noise

    // Clip the values to [0, 255] and convert back to uchar
    cv::Mat clipped_noisy_image;
    cv::threshold(noisy_image, clipped_noisy_image, 255, 255, cv::THRESH_TRUNC);
    cv::threshold(clipped_noisy_image, clipped_noisy_image, 0, 0, cv::THRESH_TOZERO);
    clipped_noisy_image.convertTo(clipped_noisy_image, CV_8U);

    return clipped_noisy_image;
}

int main() {
    int side = 256;
    int inner_square_side = 209;
    int circle_radius = 83;

    // Brightness levels for test images
    std::vector<cv::Scalar> levels = {
        cv::Scalar(0),   // Outer square
        cv::Scalar(127), // Inner square
        cv::Scalar(255)  // Circle
    };

    // Standard deviation values for noise
    std::vector<double> stddev_values = {3, 7, 15};

    // Generate test images and histograms
    std::vector<cv::Mat> test_images;
    for(const auto& level : levels) {
        std::vector<uchar> level1 = {30, 127, 220}; // Black, Gray, White for circle, inner square, and outer square respectively.
        cv::Mat image = generate_test_image(side, inner_square_side, circle_radius, level1);

        test_images.push_back(image);
    }

    // Concatenate test images horizontally
    cv::Mat test_image_row;
    cv::hconcat(test_images, test_image_row);

    // Generate noisy images and their histograms
    std::vector<cv::Mat> all_noisy_images_with_histograms;
    for(double stddev : stddev_values) {
        std::vector<cv::Mat> noisy_images_with_histograms;
        for(auto& image : test_images) {
            cv::Mat noisy_image = add_noise(image, stddev);
            cv::Mat histogram = draw_histogram(noisy_image);
            cv::Mat combined_image;
            cv::vconcat(noisy_image, histogram, combined_image);
            noisy_images_with_histograms.push_back(combined_image);
        }

        // Concatenate noisy images with histograms for the current stddev horizontally
        cv::Mat noisy_images_row;
        cv::hconcat(noisy_images_with_histograms, noisy_images_row);
        all_noisy_images_with_histograms.push_back(noisy_images_row);
    }

    // Concatenate all noisy images with histograms vertically with the test images row
    cv::Mat final_image = test_image_row;
    for (auto& noisy_row : all_noisy_images_with_histograms) {
        cv::vconcat(final_image, noisy_row, final_image);
    }

    // Show and save the final image
    cv::imshow("Test Image with Histograms and Noise", final_image);
    cv::waitKey(0);
    cv::imwrite("final_image.png", final_image);

    return 0;
}

