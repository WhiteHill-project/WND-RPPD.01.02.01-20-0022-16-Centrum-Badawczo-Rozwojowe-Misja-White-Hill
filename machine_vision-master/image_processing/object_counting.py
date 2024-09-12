from image_processing_utils import get_number_of_objects
import argparse
import sys
import json
import os


def is_valid_file(parser, path):
    if not os.path.isfile(path):
        parser.error(f'Path "{path}" does not lead to file. Please recheck.')
    else:
        return path


def main():

    parser = argparse.ArgumentParser(description='Calculate number of objects for given mask image.')
    parser.add_argument('-i', '--input-image', help='Path to the input image.', type=str, required=True)
    parser.add_argument('-codi', '--color-dictionary', help='Path to the JSON file containing dictionary containing '
                                                            'color HSV representation for a given class.',
                        type=lambda path: is_valid_file(parser, path), required=True)
    parser.add_argument('-fl', '--fluctuations', help='Percentage range around the median.', type=float, default=0.1)
    parser.add_argument('-ei', '--erode-iterations', help='Number of erode operations.', type=int, default=3)
    parser.add_argument('-d', '--display', help='Indicator if we want to display the image.', type=bool,
                        default=False)

    options = parser.parse_args()

    # Read JSON file to a dictionary.
    with open(options.color_dictionary) as file:
        color_dictionary = json.load(file)

    # Calculate the number of objects.
    number_of_objects = get_number_of_objects(
        image=options.input_image,
        color_dict=color_dictionary,
        fluctuations=options.fluctuations,
        erode_iterations=options.erode_iterations,
        display_kept=options.display
    )

    sys.stdout.write(f'Number of objects: {number_of_objects}')


if __name__ == '__main__':
    main()
