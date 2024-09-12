import argparse
import sys


def main():

    parser = argparse.ArgumentParser(description='Calculate the Harvest Index.')
    parser.add_argument('-bs', '--biomass-surface', help='Surface of the biomass.', type=float, required=True)
    parser.add_argument('-acs', '--agricultural-crop-surface', help='Surface of the agricultural crop.', type=float,
                        required=True)

    options = parser.parse_args()

    # Calculate Harvest Index (HI).
    harvest_index = 100 * options.biomass_surface / options.agricultural_crop_surface

    sys.stdout.write(f'{harvest_index:.2f}')


if __name__ == '__main__':
    main()
