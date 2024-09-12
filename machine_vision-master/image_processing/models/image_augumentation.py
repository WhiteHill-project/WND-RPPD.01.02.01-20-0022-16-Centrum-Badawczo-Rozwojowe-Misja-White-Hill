import Augmentor
from typing import List, Tuple, Dict
import numpy as np


DEFAULT_AUGMENTOR_CONFIG = {
    'flip_left_right': {
        'probability': 0.8
    },
    'flip_top_bottom': {
        'probability': 0.3
    },
    'rotate': {
        'probability': 0.8,
        'max_left_rotation': 10,
        'max_right_rotation': 10
    }
}


def create_augmentation_generator(data: List[List[np.ndarray]],
                                  configuration: Dict[str, Dict]) -> Augmentor.DataPipeline:
    """
    Function to create data augmentation generator.

    :param data: List of tuples of np.ndarray's representing image mask pairs.
    :param configuration: Dictionary containing configuration for the generator.
    :return: Generator.
    """

    # Initiate data pipeline.
    pipeline = Augmentor.DataPipeline(data)

    # Add operation based on configuration.
    for key, value in configuration.items():
        getattr(pipeline, key)(**value)

    return pipeline


def create_augmented_dataset(x_train: List[np.ndarray],
                             y_train: List[np.ndarray],
                             configuration: Dict[str, Dict],
                             number_of_examples: int) -> Tuple[List[np.ndarray], List[np.ndarray]]:
    """
    Create x_train, y_train datasets with original as well as augmented images.

    :param x_train: List of np.ndarray's representing original images.
    :param y_train: List of np.ndarray's representing mask images.
    :param configuration: Dictionary containing configuration for the generator.
    :param number_of_examples: Number of examples we want in our output datasets.
    :return: Tuple containing lists of images and masks.
    """

    # Format list of data.
    data = [list(pair) for pair in zip(x_train, y_train)]

    # Create generator
    generator = create_augmentation_generator(data=data, configuration=configuration)
    number_of_augmented_examples = number_of_examples - len(data)

    data += generator.sample(number_of_augmented_examples)

    return [image for image, _ in data], [mask for _, mask in data]
