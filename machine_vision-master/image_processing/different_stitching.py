import argparse
import cv2
import imutils
import numpy as np
import matplotlib.pyplot as plt
import imageio
import os
from typing import List, Tuple
from image_processing_utils import clahe_image
cv2.ocl.setUseOpenCL(False)


def show_image(base_image, query_image):
    """
    Function to show base and query original images.

    :param base_image: Base image.
    :param query_image: Query image.
    """

    figure, (axis_1, axis_2) = plt.subplots(nrows=1, ncols=2, constrained_layout=False, figsize=(16, 9))
    axis_1.imshow(query_image, cmap='gray')
    axis_1.set_xlabel('Query image', fontsize=14)
    axis_2.imshow(base_image, cmap='gray')
    axis_2.set_xlabel('Base image (image to be transformed)', fontsize=14)
    plt.show()


def detect_and_describe(image: np.ndarray) -> Tuple[List, np.ndarray]:
    """
    Function to create descriptor and compute description.

    :param image: Image we want to get description for.
    :return: Description of the input image.
    """

    descriptor = cv2.ORB_create()

    return descriptor.detectAndCompute(image, None)


def create_matcher(cross_check: bool):
    """
    Create the matcher for keypoints matching.

    :param cross_check: Indicator if matcher should return one or more knn matches.
    :return: Matcher.
    """

    return cv2.BFMatcher(cv2.NORM_HAMMING, crossCheck=cross_check)


def match_keypoints(features_base: np.ndarray, features_query: np.ndarray, cross_check: bool = True):
    """
    Function to match found keypoint.

    :param features_base: Features of the base image.
    :param features_query: Features of the query image.
    :param cross_check: Indicator if matcher should return one or more knn matches.
    :return: Raw, sorted matches.
    """

    matcher = create_matcher(cross_check=cross_check)
    best_matches = matcher.match(features_base, features_query)

    raw_matches = sorted(best_matches, key=lambda x: x.distance)

    return raw_matches


def match_keypoints_knn(features_base: np.ndarray, features_query: np.ndarray, ratio: float,
                        cross_check: bool = True) -> List:
    """
    Function to match found keypoint based on KNN algorithm.

    :param features_base: Features of the base image.
    :param features_query: Features of the query image.
    :param ratio: Float values indicating limit for distance in KNN algorithm.
    :param cross_check: Indicator if matcher should return one or more knn matches.
    :return: Raw matches.
    """

    matcher = create_matcher(cross_check=cross_check)
    raw_matches = matcher.knnMatch(features_base, features_query, 2)

    print(f'Raw matches (knn): {len(raw_matches)}.')

    matches = []

    for first_match, second_match in raw_matches:
        if first_match.distance < second_match.distance * ratio:
            matches.append(first_match)

    return matches


def draw_matches(base_image: imageio.core.Array, query_image: imageio.core.Array, feature_matching: str,
                 ratio: float = 0.75, cross_check: bool = True, show: bool = False) -> List:
    """
    Function to calculate and draw matches between the base and query image.

    :param base_image: Base image.
    :param query_image: Query image.
    :param feature_matching: Argument indicating which type of matching we use. Possible values: "bf", "knn".
    :param ratio: Float setting limit for distance in KNN algorithm.
    :param cross_check: Indicator if matcher should return one or more knn matches.
    :param show: Indicator telling if we want to show images in intermediate steps.
    :return: List of matches.
    """

    assert feature_matching in ['bf', 'knn'], '[INFO] '

    base_image_gray = cv2.cvtColor(base_image, cv2.COLOR_RGB2GRAY)
    query_image_gray = cv2.cvtColor(query_image, cv2.COLOR_RGB2GRAY)

    keypoints_base, features_base = detect_and_describe(base_image_gray)
    keypoints_query, features_query = detect_and_describe(query_image_gray)

    print(f'Using: {feature_matching} feature matcher.')

    if feature_matching == 'bf':
        matches = match_keypoints(features_base, features_query, cross_check=cross_check)
        image = cv2.drawMatches(base_image, keypoints_base, query_image, keypoints_query, matches[:100],
                                None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS)
    else:
        matches = match_keypoints_knn(features_base, features_query, ratio=ratio)
        image = cv2.drawMatches(base_image, keypoints_base, query_image, keypoints_query,
                                np.random.choice(matches, 100),
                                None, flags=cv2.DrawMatchesFlags_NOT_DRAW_SINGLE_POINTS)

    if show:
        plt.figure(figsize=(16, 9))
        plt.imshow(image)
        plt.show()

    return matches


def get_homography(keypoints_base: List, keypoints_query: List, matches: List, reprojection_threshold: int) -> Tuple:
    """
    Function to calculate homography matrix needed for perspective warping.

    :param keypoints_base: Detected keypoints from the base image.
    :param keypoints_query: Detected keypoints from the query image.
    :param matches: Detected matches.
    :param reprojection_threshold: Maximum allowed reprojection error to treat a point pair as an inlier.
    :return: Tuple matches, homography and status.
    """

    keypoints_base = np.float32([keypoint.pt for keypoint in keypoints_base])
    keypoints_query = np.float32([keypoint.pt for keypoint in keypoints_query])

    if len(matches) > 4:

        points_base = np.float32([keypoints_base[match.queryIdx] for match in matches])
        points_query = np.float32([keypoints_query[match.trainIdx] for match in matches])

        homography, status = cv2.findHomography(points_base, points_query, cv2.RANSAC, reprojection_threshold)

        return matches, homography, status

    else:

        return None, None, None


def panorama_correction(base_image: imageio.core.Array, query_image: imageio.core.Array, homography: np.ndarray,
                        show: bool = False) -> np.ndarray:
    """
    Function to create panoramic image.

    :param base_image: Base image.
    :param query_image: Query image.
    :param homography: Homography matrix needed to warp perspective.
    :param show: Indicator telling if we want to show images in intermediate steps.
    :return: Stitched image.
    """

    width = base_image.shape[1] + query_image.shape[1]
    height = base_image.shape[0] + query_image.shape[0]

    result = cv2.warpPerspective(base_image, homography, (width, height))
    result[0: query_image.shape[0], 0: query_image.shape[1]] = query_image

    if show:
        plt.figure(figsize=(16, 9))
        plt.imshow(result)

        plt.axis('off')
        plt.show()

    return result


def crop_corrected(image: np.ndarray, enhance: bool = False, show: bool = False) -> np.ndarray:
    """
    Function to crop stitched image.

    :param image: Stitched panoramic image.
    :param enhance: Indicator if we want to use CLAHE algorithm on the output image.
    :param show: Indicator telling if we want to show images in intermediate steps.
    :return: Cropped panoramic image.
    """

    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    threshold_image = cv2.threshold(gray_image, 0, 255, cv2.THRESH_BINARY)[1]

    contours = cv2.findContours(threshold_image.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours = imutils.grab_contours(contours)

    maximal_contour = max(contours, key=cv2.contourArea)

    x, y, width, height = cv2.boundingRect(maximal_contour)

    result = image[y:y + height, x: x + width]

    if enhance:
        result = clahe_image(result, clip_limit=6.0, grid_size=10)

    if show:
        plt.figure(figsize=(16, 9))
        plt.imshow(result)

    return result


def main():

    parser = argparse.ArgumentParser(description='Calculate number of objects for given mask image.')
    parser.add_argument('-b', '--base-image', help='Path to the images.', type=str, default=None)
    parser.add_argument('-q', '--query-image', help='Path to the query image', type=str, default=None)
    parser.add_argument('-i', '--input', help='Path to the input folder', type=str, default=None)
    parser.add_argument('-fm', '--feature-matching', help='Feature matching. Possible values: "bf", "knn".',
                        type=str, required=True)
    parser.add_argument('-s', '--show', help='Indicator if we want to show middle steps.', type=bool, default=False)
    parser.add_argument('-r', '--ratio', type=float, default=0.75)
    parser.add_argument('-rj', '--reprojection-threshold', type=int, default=4)
    parser.add_argument('-o', '--output', help='Path to output folder.', type=str, default=os.getcwd())

    options = parser.parse_args()

    images_directory = options.input
    base_image_path = options.base_image
    query_image_path = options.query_image
    ratio = options.ratio
    reprojection_threshold = options.reprojection_threshold

    if images_directory:
        print('[INFO] Input directory was provided.')
        image_paths = sorted([file for file in sorted(os.listdir(images_directory))
                              if os.path.isfile(os.path.join(images_directory, file))])
        print(f'[INFO] Input directory contains {len(image_paths)} images.')
        assert len(image_paths) >= 2, f'[INFO] Provided directory contains {len(image_paths)} images. At least two ' \
                                      f'images are needed.'
        images = [imageio.imread(path) for path in image_paths]
        images_gray = [cv2.cvtColor(image, cv2.COLOR_RGB2GRAY) for image in images]
    else:
        print('[INFO] Input directory was not provided. Base and query images paths needed.')
        assert options.base_image, '[INFO] Base image path not provided. Please check again.'
        assert options.query_image, '[INFO] Query image path not provided. Please check again.'
        print('[INFO] Both base and query images were provided.')
        image_paths = [base_image_path, query_image_path]
        images = [imageio.imread(path) for path in image_paths]
        images_gray = [cv2.cvtColor(image, cv2.COLOR_RGB2GRAY) for image in images]

    feature_matching = options.feature_matching
    show = options.show

    base_image = images[0]
    base_image_gray = images_gray[0]

    for i in range(1, len(images)):
        query_image = images[i]
        query_image_gray = images_gray[i]

        keypoints_base, _ = detect_and_describe(base_image_gray)
        keypoints_query, _ = detect_and_describe(query_image_gray)

        matches = draw_matches(base_image=base_image, query_image=query_image, feature_matching=feature_matching,
                               ratio=ratio, show=show)

        _, homography, _ = get_homography(keypoints_base=keypoints_base, keypoints_query=keypoints_query,
                                          matches=matches, reprojection_threshold=reprojection_threshold)

        panoramic_image = panorama_correction(base_image=base_image, query_image=query_image, homography=homography,
                                              show=show)

        cropped_image = crop_corrected(image=panoramic_image, show=show)
        base_image = cropped_image
        base_image_gray = cv2.cvtColor(base_image, cv2.COLOR_RGB2GRAY)

    plt.figure(figsize=(16, 9))
    plt.imshow(base_image)
    plt.show()


if __name__ == '__main__':
    main()
