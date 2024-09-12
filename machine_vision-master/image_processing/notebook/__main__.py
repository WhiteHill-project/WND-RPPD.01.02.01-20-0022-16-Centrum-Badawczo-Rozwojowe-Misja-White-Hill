import os
import cv2
import argparse
import segmentation_stalk as seg


def main():

    parser = argparse.ArgumentParser(description='Segmentation image from folder')
    parser.add_argument('-pm', '--path-model', help='Path to model segmentation.', type=str, required=True)
    parser.add_argument('-in', '--input-path', help='Path to the input image.', type=str, required=True)
    parser.add_argument('-out', '--output-path', help='Path for save mask predict.', type=str, required=True)

    options = parser.parse_args()

    segmentation_model = seg.get_model(options.path_model)
    input_dir = options.input_path
    output_dir = options.output_path

    for filename in os.listdir(input_dir):
        print(filename + ': processing start')
        image_in = cv2.resize(cv2.imread(input_dir + filename)[:, :, [2, 1, 0]],
                              (640, 800))
        transform_image = seg.transform_image(image_in)
        mask = segmentation_model.predict(transform_image)
        cut_image = seg.cut_image(image_in, mask)
        path_save = output_dir + 'mask_' + filename
        cv2.imwrite(path_save, cut_image[:, :, [2, 1, 0]])
        print(filename + ': processing final')


if __name__ == '__main__':
    main()
