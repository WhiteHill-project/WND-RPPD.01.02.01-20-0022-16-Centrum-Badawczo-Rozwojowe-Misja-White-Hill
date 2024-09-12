import cv2
import torch
import numpy as np
import torchvision.transforms as transforms


def get_model(path: str,
              device: str = 'cpu'
              ) -> object:
    """
    Function for writer segmentation model

    :param path: path to saved model
    :param device: device cpu or cuda
    :return: segmentation model
    """

    map_location = torch.device(device)
    segmentation_model = torch.load(path, map_location)
    return segmentation_model


def transform_image(image: np.ndarray) -> object:
    """
    Function for convert image to tensor

    :param image: image
    :return: tensor
    """

    tensor = transforms.ToTensor()(image)
    norm_tensor = transforms.Normalize(
        [0.485, 0.456, 0.406],
        [0.229, 0.224, 0.225])(tensor).unsqueeze(0)
    return norm_tensor.view(1, 3, 800, 640)


def cut_image(image_input: np.ndarray,
              mask_image: np.ndarray
              ) -> np.ndarray:
    """
    Function for get grabcut image

    :param image_input: image with stem
    :param mask_image: image with mask stem
    :return: grabcut image
    """

    mask = (mask_image.squeeze().cpu().numpy().round())
    _, mask_binary = cv2.threshold(mask, 0, 255, cv2.THRESH_BINARY)
    mask_binary = np.where((mask_binary == 0), 0, 1).astype('uint8')
    result_image = image_input * mask_binary[:, :, np.newaxis]
    background = image_input - result_image
    # Change all pixels in the background that are not black to white
    background[np.where((background >= [0, 0, 0]).all(axis=2))] = [255, 255, 255]
    # Add the background and the image
    final_image = background + result_image
    return final_image



