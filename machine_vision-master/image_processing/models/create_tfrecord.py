from tfrecords_utils import read_images, write_image_annotation_pairs_to_tfrecord
import argparse


def main():

    parser = argparse.ArgumentParser(description='Create TFRecord file.')
    parser.add_argument('-i', '--input', help='Path to the images directory.', type=str, required=True)
    parser.add_argument('-o', '--output', help='Path to the output TFRecord file.', type=str, required=True)
    options = parser.parse_args()

    # Get list of image-mask paths pairs.
    image_to_mask_paths = read_images(options.input)

    # Create TFRecord file.
    write_image_annotation_pairs_to_tfrecord(image_to_mask_paths, options.output)


if __name__ == '__main__':
    main()
