from PIL import Image
import numpy as np
import tensorflow as tf
import os
from typing import List, Tuple


def read_images(images_directory: str) -> []:
    """
    Function to read image and mask items from data directory and creating a list of image-mask paths pairs.

    :param images_directory: Path to the data directory.
    :return: List of image-mask paths pairs.
    """

    # Get the subdirectories names list from given directory.
    subdirectories_list = [file for file in os.listdir(images_directory) if
                           os.path.isdir(os.path.join(images_directory, file))]

    image_mask_pairs_list = []

    # Iterate over the subdirectories.
    for subdirectory in subdirectories_list:
        subdirectory_path = os.path.join(images_directory, subdirectory)

        # Get original and mask files directories paths.
        image_file_directory = os.path.join(subdirectory_path, 'images')
        mask_file_directory = os.path.join(subdirectory_path, 'masks')

        # Crete full original and mask paths.
        file_path = [file for file in sorted(os.listdir(image_file_directory))
                     if os.path.isfile(os.path.join(image_file_directory, file))]
        if len(file_path) == 0:
            raise ValueError(f'{os.path.basename(subdirectory_path)} original image is not provided.')
        else:
            file_path = file_path[0]
        image_full_path = os.path.join(image_file_directory, file_path)
        mask_path = [file for file in sorted(os.listdir(mask_file_directory))
                     if os.path.isfile(os.path.join(mask_file_directory, file))]
        if len(mask_path) == 0:
            raise ValueError(f'{os.path.basename(subdirectory_path)} mask image is not provided.')
        else:
            mask_path = mask_path[0]
        mask_full_path = os.path.join(mask_file_directory, mask_path)

        # Append image-mask path pair to the list.
        image_mask_pairs_list.append((image_full_path, mask_full_path))

    return image_mask_pairs_list


def _bytes_feature(value):
    return tf.train.Feature(bytes_list=tf.train.BytesList(value=[value]))


def _int64_feature(value):
    return tf.train.Feature(int64_list=tf.train.Int64List(value=[value]))


def write_image_annotation_pairs_to_tfrecord(image_mask_pairs_list: List[Tuple[str, str]], tfrecord_file_path: str):
    """
    The function creates tfrecord file given the list of original-mask paths pairs.

    :param image_mask_pairs_list: List containing original-mask paths pairs.
    :param tfrecord_file_path: Path to the tfrecord file.
    """

    # Initiate TFRecordWriter in given path.
    writer = tf.python_io.TFRecordWriter(tfrecord_file_path)

    # Iterate over original-mask paths pairs.
    for image_path, mask_path in image_mask_pairs_list:

        # Import original image and mask.
        image = np.array(Image.open(image_path))
        mask = np.array(Image.open(mask_path))

        # Unpack image height and width.
        height = image.shape[0]
        width = image.shape[1]

        # Create string representations of image and mask.
        image_raw = image.tostring()
        mask_raw = mask.tostring()

        # Transform image and mask example into TFRecord example.
        example = tf.train.Example(features=tf.train.Features(feature={
            'height': _int64_feature(height),
            'width': _int64_feature(width),
            'image_raw': _bytes_feature(image_raw),
            'mask_raw': _bytes_feature(mask_raw)})
        )

        # Write the example into a TFRecord file.
        writer.write(example.SerializeToString())

    writer.close()


def read_image_annotation_pairs_from_tfrecord(tfrecord_file_path: str) -> []:
    """
    Function to read TFRecord file into list of image-mask pairs.

    :param tfrecord_file_path: Path to the TFRecord file.
    :return: List containing image-mask pairs.
    """

    image_mask_pairs = []

    # Initiate iterator over TFRecord file.
    record_iterator = tf.python_io.tf_record_iterator(path=tfrecord_file_path)

    # Iterate over the records.
    for string_record in record_iterator:
        example = tf.train.Example()
        example.ParseFromString(string_record)

        # Extract height, width, image and mask strings from the record.
        height = int(example.features.feature['height']
                     .int64_list
                     .value[0])
        width = int(example.features.feature['width']
                    .int64_list
                    .value[0])
        image_string = (example.features.feature['image_raw']
                        .bytes_list
                        .value[0])
        mask_string = (example.features.feature['mask_raw']
                       .bytes_list
                       .value[0])

        # Transform and reshape image and mask strings.
        image_1d = np.fromstring(image_string, dtype=np.uint8)
        image = image_1d.reshape((height, width, -1))

        mask_1d = np.fromstring(mask_string, dtype=np.uint8)
        mask = mask_1d.reshape((height, width))

        # Append image-mask pair to list.
        image_mask_pairs.append((image, mask))

    return image_mask_pairs


def read_tfrecord_and_decode_into_image_annotation_pair_tensors(tfrecord_filenames_queue: str) -> ():
    """
    The function accepts tfrecord filenames queue as an input which is usually can be created using
    tf.train.string_input_producer() where filename is specified with desired number of epochs. This function takes
    queue produced by aforementioned tf.train.string_input_producer() and defines tensors converted from raw binary
    representations into reshaped image/annotation tensors.


    :param tfrecord_filenames_queue: String queue object from tf.train.string_input_producer().
    :return: Tuple of image/mask tensors.
    """

    # Initiate TFRecordReader.
    reader = tf.TFRecordReader()

    # Get the serialized example from TFRecord.
    _, serialized_example = reader.read(tfrecord_filenames_queue)

    # Parse the serialized example.
    features = tf.parse_single_example(
        serialized_example,
        features=dict(height=tf.FixedLenFeature([], tf.int64), width=tf.FixedLenFeature([], tf.int64),
                      image_raw=tf.FixedLenFeature([], tf.string), mask_raw=tf.FixedLenFeature([], tf.string)))

    # Decode raw image and mask.
    image = tf.decode_raw(features['image_raw'], tf.uint8)
    mask = tf.decode_raw(features['mask_raw'], tf.uint8)

    # Extract height and width.
    height = tf.cast(features['height'], tf.int32)
    width = tf.cast(features['width'], tf.int32)

    # Get the image and mask shapes.
    image_shape = tf.stack([height, width, 3])
    mask_shape = tf.stack([height, width, 1])

    # Finally reshape image and mask.
    image = tf.reshape(image, image_shape)
    mask = tf.reshape(mask, mask_shape)

    return image, mask


def unpack_datasets(image_mask_pairs: []) -> ():
    """
    Function to unpack pairs list to x_train and y_train datasets.

    :param image_mask_pairs: List containing image-mask pairs.
    :return: Tuple containing image and mask lists.
    """

    # Initiate the image and mask lists.
    x_train = []
    y_train = []

    # Iterate over the pairs.
    for i in range(len(image_mask_pairs)):

        # Unpack the image and mask from pairs.
        image, mask = image_mask_pairs[i][0]

        # Create image and mask lists with examples for training.
        x_train.append(image)
        y_train.append(mask)

    return x_train, y_train
