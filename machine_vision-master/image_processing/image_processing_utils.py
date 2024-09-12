from matplotlib import pyplot as plt
import cv2
import imutils
import numpy as np
from statistics import median
from os.path import basename
import re


def clahe_image(image: np.ndarray, clip_limit: float = 6.0, grid_size: int = 10) -> np.ndarray:
    """
    Custom function to process input image using Contrast Limited Adaptive Histogram Equalization (CLAHE).

    :param image: Input image we want to process.
    :param clip_limit: Threshold for contrast limiting.
    :param grid_size: Size of grid for histogram equalization. Input image will be divided into equally sized
                      rectangular tiles, grid_size defines the number of tiles in row and column
    :return: Processed input image in RGB colorspace.
    """

    # Convert input image to LAB color space. ('L' stands for lightness from black (0) to white (100), 'a' from green
    # (-) to red (+), 'b' from blue (-) to yellow (+).)
    lab_image = cv2.cvtColor(image, cv2.COLOR_RGB2LAB)

    # Split image into planes.
    lab_planes = cv2.split(lab_image)

    # Initiate CLAHE instance implemented in OpenCV library.
    clahe = cv2.createCLAHE(clipLimit=clip_limit, tileGridSize=(grid_size, grid_size))

    # Use CLAHE on first of the planes.
    lab_planes[0] = clahe.apply(lab_planes[0])

    # Merge planes back into image.
    lab_image_merged = cv2.merge(lab_planes)

    # Convert image back to RGB color space.
    rgb_image = cv2.cvtColor(lab_image_merged, cv2.COLOR_LAB2RGB)

    return rgb_image


def _display_single(image: np.ndarray, title: str = None, cmap: str = 'gray'):
    """
    Function to display single image.

    :param image: Input image.
    :param title: Optional title for the displayed image.
    :param cmap: Colormap for displayed image.
    """

    # Get width and height of the image.
    image_width, image_height = image.shape[0], image.shape[1]

    # Get current figure.
    figure = plt.gcf()

    # Get Dots Per Inch from the figure.
    dots_per_inch = figure.get_dpi()

    # Set shape to the figure.
    figure.set_size_inches(image_width / float(dots_per_inch), image_height / float(dots_per_inch))

    # If image has three channels then show image, else set cmap to given, e.g. for grayscale image.
    if len(image.shape) == 3:
        plt.imshow(image)
    else:
        plt.imshow(image, cmap=cmap)

    # Show image title if given.
    if title:
        plt.title(title)


def display(image, title: str = None):
    """
    Wrapper around _display_single function (?).

    :param image: Input image.
    :param title: Additional title for the image.
    """

    _display_single(image, title)
    plt.show()


def display_file(file_path: str, title: str = None) -> np.ndarray:
    """
    Function to load and display the image from file_path.

    :param file_path: Path to the file.
    :param title: Additional title for input file.
    :return: Loaded image from the input path.
    """

    # Read image given by the path and convert color space to RGB, because OpenCV default color space is BGR.
    image = cv2.imread(file_path)
    image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # If cant find image specified by input path, raise error.
    if image is None:
        raise IOError(f"Could not open {file_path}.")

    # If title was not given, use basename of the image as title.
    if not title:
        title = basename(file_path)
    display(image, title)

    return image


def combine_multiple_masks(masks_list: []) -> np.ndarray:
    """
    Function to combine multiple mask images into single image.
    :param masks_list: List of the np.ndarray's representing mask images.
    :return: Combined masks image.
    """

    # Iterate over masks in a list and combine them.
    combined_image = masks_list[0]
    for mask in masks_list[1:]:
        combined_image += mask

    return combined_image


def color_image_subset(image, pixels_list: [], color: int = 255) -> np.ndarray:
    """
    Function to draw rough contours on image.

    :param image: Input image.
    :param pixels_list: List of the pixels.
    :param color: A color we want to use for drawing.
    :return: Image which changed pixels.
    """

    # Set color of given pixels in pixels_list to the one represented by 'color' variable.
    for pixel in pixels_list:
        image[pixel[0][1], pixel[0][0]] = color

    return image


def count_contours_above_median(input_image, fluctuations: float, display_kept: bool = False) -> int:
    """
    Function to find and count contours with length above median +- fluctuations.

    :param input_image: Mask image.
    :param fluctuations: Percentage range around the median.
    :param display_kept: An indicator if we want to display mask with roughly drawn kept contours.
    :return: Number of contours after processing.
    """

    # Get contours and each pixel for each contour.
    contours = cv2.findContours(input_image, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours = imutils.grab_contours(contours)

    # Get list of contour lengths, calculate median from it and then calculate limit for keeping the contour.
    contour_lengths = [len(contour) for contour in contours]
    contour_lengths_median = median(contour_lengths)
    contour_lengths_crop_limit = int(contour_lengths_median * (1 - fluctuations))

    # Initiate list in which we will store indexes of contours we want to keep.
    keep_indexes = []

    # Iterate over the contours.
    for i in range(len(contours)):
        contour_size = len(contours[i])
        if contour_size >= contour_lengths_crop_limit:
            keep_indexes.append(i)

    # If display_kept=True then display edge of contours we kept.
    if display_kept:

        # Create empty mask.
        mask = np.zeros(input_image.shape, dtype=input_image.dtype)

        # Get pixels of kept contours.
        kept_contours_pixels = [contour for i, contour in enumerate(contours) if i in keep_indexes]

        # Draw each contour on mask.
        for kept_contour in kept_contours_pixels:
            mask = color_image_subset(mask, kept_contour)

        # Display the mask with drawn contours.
        display(mask)

    return len(keep_indexes)


def get_class_mask(image, color_dict: {}) -> np.ndarray:
    """
    Function to create a mask with a single class for given segmentation image.

    :param image: Input image.
    :param color_dict: A dictionary containing HSV representation of color for a given class.
    :return: Mask image for given class.
    """

    if isinstance(image, str):
        image = cv2.imread(image)

    # Unpack the color_dict.
    hue_range = color_dict['hue']
    saturation_range = color_dict['saturation']
    value_range = color_dict['value']

    # Change image channels to HSV and create mask for color specified in color_dict.
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv_image, (hue_range[0], saturation_range[0], value_range[0]),
                       (hue_range[1], saturation_range[1], value_range[1]))

    return mask


def get_number_of_objects(image, color_dict: {}, fluctuations: float, erode_iterations: int = 3,
                          display_kept: bool = False) -> int:
    """
    Function to calculate and return a number of objects in a mask image after some morphological transformations.

    :param image: Input image.
    :param color_dict: Dictionary containing HSV representation of a class.
    :param fluctuations: Percentage range around the median.
    :param erode_iterations: Number of erode operations.
    :param display_kept: An indicator if we want to display mask with roughly drawn kept contours.
    :return: Number of the objects.
    """

    # Check if passed image is np.array or just image string.
    if isinstance(image, str):
        image = cv2.imread(image)

    # Get mask for specified class.
    mask = get_class_mask(image, color_dict)

    # Erode the image erode_iterations times and then use opening.
    kernel = np.ones((5, 5), np.uint8)
    eroded_image = cv2.erode(mask, kernel, iterations=erode_iterations)
    opening_image = cv2.morphologyEx(eroded_image, cv2.MORPH_OPEN, kernel)

    # Get number of objects in the image.
    number_of_objects = count_contours_above_median(opening_image, fluctuations, display_kept)

    return number_of_objects


def get_mask_surface_area(mask, pixel_to_cm: float, morphological_operations: bool = True, display_mask: bool = False) \
        -> float:
    """
    Function to calculate the surface area for the mask image.

    :param mask: Mask image.
    :param pixel_to_cm: Predictor for pixels to centimeters.
    :param morphological_operations: Indicator if we want to use morphological transformations.
    :param display_mask: An indicator if we want to display mask with roughly drawn kept contours.
    :return: Approximate area of the surface.
    """

    # Check if passed image is np.array or just image string.
    if isinstance(mask, str):
        mask = cv2.imread(mask)

    # If morphological_operations == True then use opening and closing.
    if morphological_operations:
        kernel = np.ones((5, 5), np.uint8)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

    # If display_mask == True, then display the mask image.
    if display_mask:
        display(mask)

    unique, counts = np.unique(mask, return_counts=True)

    surface_area = counts[1] * pixel_to_cm ** 2

    return surface_area


def concatenate_images(image_left: np.ndarray, image_right: np.ndarray) -> np.ndarray:
    """
    Function to concatenate two images into one.

    :param image_left: Left image.
    :param image_right: Image to concatenate from right side.
    :return: Concatenated image.
    """

    # Get height and width.
    height_1, width_1 = image_left.shape[:2]
    height_2, width_2 = image_right.shape[:2]

    # Create empty image with right shape.
    concatenated_image = np.zeros((max(height_1, height_2), width_1 + width_2, 3), np.uint8)

    # Fill empty matrix and concatenate images.
    concatenated_image[:height_1, 0:width_1, :3] = image_left
    concatenated_image[:height_2, width_1: width_1 + width_2, :3] = image_right

    return concatenated_image


def stitch_panorama_image(images_list: [], side: str, ratio: float) -> np.ndarray:
    """
    Function to stitch list of images into a single 'panoramic' image.

    :param images_list: List of images we want to stitch together.
    :param side: Side to which camera was facing when taking the images.
    :param ratio: Ratio of image width we want to crop out from overlapping images.
    :return: Stitched 'panoramic' image.
    """

    # If side == 'right' then we have to reverse list order.
    if side == 'right':
        images_list = images_list[::-1]

    # Initiate stitched_image and iterate over the images in the list.
    stitched_image = None
    for i, image in enumerate(images_list):

        # If i == 0, then stitched_image becomes first image in the list.
        if i == 0:
            stitched_image = images_list[i]
        else:
            image = images_list[i]

            # Get image width and crop this image.
            image_width = image.shape[1]
            image = image[:, int(ratio * image_width):]

            # Concatenate stitched_image and cropped image.
            stitched_image = concatenate_images(stitched_image, image)

    return stitched_image


def stitch_image_name(configuration_dictionary: {}, image_type: str):
    """
    Function to create stitched image name based on input configuration_dictionary.

    :param configuration_dictionary: Dictionary with configuration.
    :param image_type: Type of the image, either 'original' or 'mask'.
    :return: String representing stitched image name.
    """

    greenhouse_id = configuration_dictionary['greenhouse_id']
    row_id = configuration_dictionary['row_id']
    side = configuration_dictionary['side']
    date = configuration_dictionary['date']

    return f'{greenhouse_id}_{row_id}_{side}_{date}_{image_type}_stitched.jpeg'


def configuration_to_regex_pattern(configuration: {}) -> str:
    """
    Function to create regex pattern based on the input configuration.

    :param configuration: Dictionary containing configuration for creating regex pattern.
    :return: Compiled regex pattern.
    """

    # Extract values from configuration dictionary. Default values are set to match all combinations for each variable.
    greenhouse_id = configuration.get('greenhouse_id', '[A-Z]+\d+')
    row_id = configuration.get('row_id', '\d+')
    side = configuration.get('side', '[a-z]+')
    sequence_id = configuration.get('sequence_id', '\d+')
    date = configuration.get('date', '[A-Z]+\d+')
    image_type = configuration.get('image_type', '[a-z]+')

    # Compile regex pattern.
    regex_pattern = f'{greenhouse_id}_{row_id}_{side}_{sequence_id}_{date}_{image_type}.jpeg'

    return r'{}'.format(regex_pattern)


def match_pattern(regex_pattern: str, strings_list: []) -> []:
    """
    Function to create list of all strings matching regex_pattern from strings_list.

    :param regex_pattern: Regex pattern used for matching strings.
    :param strings_list: List of strings we want to match.
    :return: List of matches.
    """

    # Transform list of strings into one multilined string.
    multilined_string = '\n'.join(strings_list)

    return re.findall(regex_pattern, multilined_string, re.MULTILINE)
