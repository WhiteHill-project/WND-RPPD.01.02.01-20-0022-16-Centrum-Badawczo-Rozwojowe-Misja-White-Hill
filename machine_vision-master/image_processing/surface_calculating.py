from image_processing_utils import get_mask_surface_area
import argparse
import sys


def main():

    parser = argparse.ArgumentParser(description='Calculate surface of the objects from the mask.')
    parser.add_argument('-i', '--input-image', help='Path to the input mask image.', type=str, required=True)
    parser.add_argument('-ptcr', '--pixel-to-cm-ratio', help='Pixel to cm ratio.', type=float, required=True)
    parser.add_argument('-mo', '--morphological-operations', help='Indicator if we want to use opening and closing '
                                                                  'operations.', type=bool, default=True)
    parser.add_argument('-dm', '--display-mask', help='Indicator if we want to display the mask image.', type=bool,
                        default=False)
    parser.add_argument('-ot', '--object-type', help='Optional variable stating the type of the object.', type=str,
                        default=None)
    parser.add_argument('-wr', '--weight-ratio', help='Optional variable stating ratio of weight to volume, in g/cm^2.',
                        type=float, default=None)

    options = parser.parse_args()

    # Extract variables from argparse.
    weight_ratio = options.weight_ratio

    # Calculate the surface of the objects.
    objects_surface = get_mask_surface_area(
        mask=options.input_image,
        pixel_to_cm=options.pixel_to_cm_ratio,
        morphological_operations=options.morphological_operations,
        display_mask=options.display_mask
    )

    if options.object_type == 'fruit' and weight_ratio:
        sys.stdout.write(f'{objects_surface:.4f} {(objects_surface * weight_ratio):.8f}')
    else:
        sys.stdout.write(f'{objects_surface:.4f}')


if __name__ == '__main__':
    main()
