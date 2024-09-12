from image_processing_utils import configuration_to_regex_pattern, match_pattern, stitch_panorama_image, \
    stitch_image_name
from models.tfrecords_utils import read_images
import argparse
import json
import os
import cv2


def main():

    parser = argparse.ArgumentParser(description='Stitch images from single row in a single image.')
    parser.add_argument('-i', '--input-directory', help='Path to the images directory.', type=str, required=True)
    parser.add_argument('-cp', '--configuration-path', help='Path to the JSON configuration file.', type=str,
                        required=True)
    parser.add_argument('-o', '--output-directory', help='Path to stitched images directory.', type=str, required=True)

    options = parser.parse_args()

    # Extract variables from argparse.
    output_directory = options.output_directory

    # Load configuration file to a dictionary.
    with open(options.configuration_path) as file:
        configuration_dictionary = json.load(file)

    # Create regex pattern from configuration dictionary.
    regex_pattern = configuration_to_regex_pattern(configuration_dictionary)

    # Extract names of subdirectories.
    image_mask_pairs_list = read_images(options.input_directory)

    # Extract images and masks paths.
    images_paths_list = [image for image, _ in image_mask_pairs_list]
    masks_paths_list = [mask for _, mask in image_mask_pairs_list]

    # Get all images and masks fitting regex_pattern.
    images_matched_patterns = match_pattern(regex_pattern, images_paths_list)
    masks_matched_patterns = match_pattern(regex_pattern, masks_paths_list)

    # Get list of full paths for matched images and masks.
    images_matches_full_paths = [path for path in images_paths_list if os.path.basename(path) in
                                 images_matched_patterns]
    masks_matches_full_paths = [path for path in masks_paths_list if os.path.basename(path) in masks_matched_patterns]

    # Load images using OpenCV library.
    loaded_images = [cv2.cvtColor(cv2.imread(path), cv2.COLOR_BGR2RGB) for path in images_matches_full_paths]
    loaded_masks = [cv2.cvtColor(cv2.imread(path), cv2.COLOR_BGR2RGB) for path in masks_matches_full_paths]

    # Create and save 'panoramic' image and mask.
    side = configuration_dictionary['side']
    ratio = configuration_dictionary['ratio']
    image_stitched = stitch_panorama_image(loaded_images, side, ratio)
    mask_stitched = stitch_panorama_image(loaded_masks, side, ratio)

    # Save stitched image and mask to appropriate directories.
    stitched_images_folder_path = os.path.join(output_directory, 'images')
    stitched_masks_folder_path = os.path.join(output_directory, 'masks')
    stitched_image_name = stitch_image_name(configuration_dictionary, 'original')
    stitched_mask_name = stitch_image_name(configuration_dictionary, 'mask')

    cv2.imwrite(os.path.join(stitched_images_folder_path, stitched_image_name), image_stitched)
    cv2.imwrite(os.path.join(stitched_masks_folder_path, stitched_mask_name), mask_stitched)


if __name__ == '__main__':
    main()
