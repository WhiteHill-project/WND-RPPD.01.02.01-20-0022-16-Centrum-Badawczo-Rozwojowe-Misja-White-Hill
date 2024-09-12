from typing import List, Tuple, Dict
import math
from pyzbar.pyzbar import decode
import numpy as np
import cv2


def get_image(image_path: str) -> np.ndarray:
    """
    Function for reading image

    :param image_path: path to image
    :return: image in rgb
    """

    return cv2.imread(image_path)[:, :, [2, 1, 0]]


def get_mask_image(mask_path: str) -> np.ndarray:
    """
    Function for reading mask image

    :param mask_path: path to mask image  
    :return: mask in grayscale
    """

    return cv2.imread(mask_path, 0)


def qr_code_detect(image: np.ndarray) -> List:
    """
    Decode qr-code to data, polygon, bouding box points

    :param image: image of stem with gr code
    :return: list with information about qr-code
    """

    return decode(image)


def cut_image(image: np.ndarray,
              mask_image: np.ndarray
              ) -> np.ndarray:
    """
    Function for get grabcut image

    :param image: image with stem
    :param mask_image: image with mask stem
    :return: grabcut image
    """

    mask_image = np.where((mask_image == 0), 0, 1).astype('uint8')
    image = image*mask_image[:, :, np.newaxis]
    
    return image


def draw_bbox(image: np.ndarray,
              polygon: List
              ) -> np.ndarray:
    """
    Function helper for draw polygon qr-code

    :param image: image with qr-code
    :param polygon: qr-code polygon points
    :return: image with drawing polygon qr-code
    """
    
    cv2.line(image, (polygon[0][0], polygon[0][1]),
             (polygon[1][0], polygon[1][1]), (0, 0, 255), 3)
    cv2.line(image, (polygon[1][0], polygon[1][1]),
             (polygon[2][0], polygon[2][1]), (0, 0, 255), 3)
    cv2.line(image, (polygon[2][0], polygon[2][1]),
             (polygon[3][0], polygon[3][1]), (0, 0, 255), 3)
    cv2.line(image, (polygon[3][0], polygon[3][1]),
             (polygon[0][0], polygon[0][1]), (0, 0, 255), 3)

    return image


def detect_edge(mask_image: np.ndarray) -> np.ndarray:
    """
    Function for get image with stem contour

    :param mask_image: image with stem
    :return: image with stem contour
    """

    gray_image = cv2.cvtColor(mask_image, cv2.COLOR_BGR2GRAY)
    blur_image = cv2.blur(gray_image, (3, 3))
    canny_image = cv2.Canny(blur_image, 75, 100, apertureSize=3)
    dilate_image = cv2.dilate(canny_image, None, iterations=1)
    erode_image = cv2.erode(dilate_image, None, iterations=1)
    return erode_image


def create_dict_line_qr(qrs_data: List) -> Dict:
    """
    Function for gets dict with points line polygon qr-code

    :param qrs_data: list with information about qr-code
    :return: dict with lines polygon qr-code
    """
    
    polygon = qrs_data.polygon
    points = [polygon[0],
              polygon[1],
              polygon[2],
              polygon[3]]
    if points[3][1] > 0.5 * \
            (qrs_data[0].rect.top - qrs_data[0].rect.height):
        point = points.pop(3)
        points.insert(0, point)
    return {'left_line':
            [(points[0][0], points[0][1]), (points[1][0], points[1][1])],
            'down_line':
            [(points[1][0], points[1][1]), (points[2][0], points[2][1])],
            'right_line':
            [(points[2][0], points[2][1]), (points[3][0], points[3][1])],
            'up_line':
            [(points[3][0], points[3][1]), (points[0][0], points[0][1])]
            }


def calculate_points_parallel_line(dict_line_qr: dict,
                                   distance: int
                                   ) -> dict:
    """
    Function for get parallels lines up and down  polygon qr-code

    :param dict_line_qr: dict with lines qr-code
    :param distance: distance for parallels lines
    :return: dict with points parallels lines
    """

    up_line_start_x, up_line_start_y, up_line_end_x, up_line_end_y = \
        dict_line_qr.get('up_line')[1][0], \
        dict_line_qr.get('up_line')[1][1], \
        dict_line_qr.get('up_line')[0][0], \
        dict_line_qr.get('up_line')[0][1]

    down_line_start_x, down_line_start_y, down_line_end_x, down_line_end_y = \
        dict_line_qr.get('down_line')[0][0], \
        dict_line_qr.get('down_line')[0][1], \
        dict_line_qr.get('down_line')[1][0], \
        dict_line_qr.get('down_line')[1][1]

    return {
        'parallel_line_up':
            [[up_line_start_x, up_line_start_y - distance],
             [up_line_end_x, up_line_end_y - distance]],
        'parallel_line_down':
            [[down_line_start_x, down_line_start_y + distance],
             [down_line_end_x, down_line_end_y + distance]]
    }


def extend_line(start_x: float,
                start_y: float,
                end_x: float,
                end_y: float,
                length: float
                ) -> Tuple[float, float]:
    """
    Function for calculate extend height points for lines

    :param start_x: start x point line
    :param start_y: start y point line
    :param end_x: end x point line
    :param end_y: end y point line
    :param length: new length line
    :return: end points of an elongated line
    """

    length_line = math.sqrt((start_x - end_x) ** 2 + (start_y - end_y) ** 2)

    extend_x = end_x + (end_x - start_x) / length_line * length

    extend_y = end_y + (end_y - start_y) / length_line * length

    return extend_x, extend_y


def calculate_points_extend_parallel_line(dict_parallel_line_qr: dict,
                                          length_line: int
                                          ) -> dict:
    """
    Function for calculate extend height points for parallels lines

    :param dict_parallel_line_qr: dict with points parallels lines
    :param length_line: height elongated line
    :return:
    """

    start_x_up, start_y_up = \
        dict_parallel_line_qr.get('parallel_line_up')[1][0], \
        dict_parallel_line_qr.get('parallel_line_up')[1][1]
    end_x_up, end_y_up = \
        dict_parallel_line_qr.get('parallel_line_up')[0][0], \
        dict_parallel_line_qr.get('parallel_line_up')[0][1]
    ext_x_up, ext_y_up = extend_line(start_x_up,
                                     start_y_up,
                                     end_x_up,
                                     end_y_up,
                                     length_line)
    start_x_down, start_y_down = \
        dict_parallel_line_qr.get('parallel_line_down')[1][0], \
        dict_parallel_line_qr.get('parallel_line_down')[1][1]
    end_x_down, end_y_down = \
        dict_parallel_line_qr.get('parallel_line_down')[0][0], \
        dict_parallel_line_qr.get('parallel_line_down')[0][1]   
    ext_x_down, ext_y_down = extend_line(
        start_x_down,
        start_y_down,
        end_x_down,
        end_y_down,
        length_line)
    new_start_x_up, new_start_y_up = \
        (start_x_up + ext_x_up) / 2, \
        (start_y_up + ext_y_up) / 2
    new_start_x_down, new_start_y_down = \
        (start_x_down + ext_x_down) / 2, \
        (start_y_down + ext_y_down) / 2
    return {'parallel_up':
            [[new_start_x_up, new_start_y_up], [ext_x_up, ext_y_up]],
            'parallel_down':
            [[new_start_x_down, new_start_y_down], [ext_x_down, ext_y_down]]
            }


def draw_lines(image: np.ndarray,
               parallel_line: Dict
               ) -> None:
    """
    Function helper for draw lines in binary images

    :param image: image with contour stem
    :param parallel_line: dict with points parallels lines
    :return: None
    """

    start_x_up, start_y_up = \
        parallel_line.get('parallel_up')[0][0], \
        parallel_line.get('parallel_up')[0][1]
    end_x_up, end_y_up = \
        parallel_line.get('parallel_up')[1][0], \
        parallel_line.get('parallel_up')[1][1]
    start_x_down, start_y_down = \
        parallel_line.get('parallel_down')[0][0], \
        parallel_line.get('parallel_down')[0][1]
    end_x_down, end_y_down = \
        parallel_line.get('parallel_down')[1][0], \
        parallel_line.get('parallel_down')[1][1]
    cv2.line(image, (int(start_x_up), int(start_y_up)),
             (int(end_x_up), int(end_y_up)), (255, 255, 255), 1)
    cv2.line(image, (int(start_x_down), int(start_y_down)),
             (int(end_x_down), int(end_y_down)), (255, 255, 255), 1)


def bresenhams_line_algorithm(start: Tuple[int, int],
                              end: Tuple[int, int]
                              ) -> List:
    """
    Function for calculate alls points line

    :param start: x,y start
    :param end: x,y and
    :return: list with all points line
    """
    # Setup initial conditions
    x_1, y_1 = start
    x_2, y_2 = end
    d_x = x_2 - x_1
    d_y = y_2 - y_1

    # Determine how steep the line is
    is_steep = abs(d_y) > abs(d_x)

    # Rotate line
    if is_steep:
        x_1, y_1 = y_1, x_1
        x_2, y_2 = y_2, x_2

    # Swap start and end points if necessary and store swap state
    swapped = False
    if x_1 > x_2:
        x_1, x_2 = x_2, x_1
        y_1, y_2 = y_2, y_1
        swapped = True

    # Recalculate differentials
    d_x = x_2 - x_1
    d_y = y_2 - y_1

    # Calculate error
    error = int(d_x / 2.0)
    y_step = 1 if y_1 < y_2 else -1

    # Iterate over bounding box generating points between start and end
    y_new = y_1
    points = []
    for x_new in range(x_1, x_2 + 1):
        coord = (y_new, x_new) if is_steep else (x_new, y_new)
        points.append(coord)
        error -= abs(d_y)
        if error < 0:
            y_new += y_step
            error += d_x

    # Reverse the list if the coordinates were swapped
    if swapped:
        points.reverse()
    return points


def get_all_points_line(points_lines: Dict) -> Dict:
    """
    Function for get list with all points line 

    :param points_lines: start x,y and end x,y
    :return: dict with list all points for parallels up and down lines
    """

    start_points_line_up = (
        int(points_lines.get('parallel_up')[0][0]),
        int(points_lines.get('parallel_up')[0][1]))
    end_points_line_up = (
        int(points_lines.get('parallel_up')[1][0]),
        int(points_lines.get('parallel_up')[1][1]))
    start_points_line_down = (
        int(points_lines.get('parallel_down')[0][0]),
        int(points_lines.get('parallel_down')[0][1]))
    end_points_line_down = (
        int(points_lines.get('parallel_down')[1][0]),
        int(points_lines.get('parallel_down')[1][1]))
    points_line_up = bresenhams_line_algorithm(
        start_points_line_up,
        end_points_line_up)
    points_line_down = bresenhams_line_algorithm(
        start_points_line_down,
        end_points_line_down)

    return {
            'line_up': points_line_up,
            'line_down': points_line_down
    }


def detect_points_intersection(image: np.ndarray,
                               list_points_line: List
                               ) -> List:
    """
    Function for detect points intersection parallels elongated line with contour stem

    :param image: binary image with stem contour
    :param list_points_line: all points parallels elongateds lines
    :return:
    """

    points_intersection = [point for point in list_points_line
                           if image[point[1] - 1, point[0]] == 255
                           or image[point[1] + 1, point[0]] == 255
                           ]

    return points_intersection


def calculate_length(start: Tuple,
                     end: Tuple
                     ) -> float:
    """
    Calculate length

    :param start: start x,y
    :param end: end x,y
    :return: length
    """

    return math.sqrt(
        (start[0] - end[0]) ** 2 + (start[1] - end[1]) ** 2
    )


def px_per_mm(points_1: Tuple[float, float],
              points_2: Tuple[float, float],
              size_qr_mm: float = 50.0
              ) -> float:
    """
    Function for calculate pixels per mm

    :param points_1: left up points polygon qr-code
    :param points_2: right down points polygon qr-code
    :param size_qr_mm: real side size qr_code in mm
    :return: pixels per mm
    """

    return math.sqrt(2 * size_qr_mm * size_qr_mm /
                     ((points_1[0] - points_2[0])**2 + (points_1[1] - points_2[1])**2))


def get_stem_thickness(image: np.ndarray,
                       mask_image: np.ndarray,
                       rect: int,
                       polygon: List,
                       size_qr_mm: float
                       ) -> float:
    """
    Main function for calculate stem thickness

    :param image: image
    :param mask_image: mask image
    :param rect:
    :param polygon:
    :param size_qr_mm:
    :return:
    """

    cut_image_stem = cut_image(image, mask_image)

    image_canny = detect_edge(cut_image_stem)

    dict_line_qr = create_dict_line_qr(polygon)

    parallel_line = calculate_points_parallel_line(
        dict_line_qr,
        30
    )

    extends_line = calculate_points_extend_parallel_line(
        parallel_line,
        400
    )

    draw_lines(image_canny, extends_line)

    all_points_lines = get_all_points_line(extends_line)

    detect_points_line_up = detect_points_intersection(
        canny_image,
        all_points_lines.get('line_up')
    )

    detect_points_line_down = detect_points_intersection(
        canny_image,
        all_points_lines.get('line_down')
    )

    mm_in_pixel = rect / size_qr_mm

    thickness_up = calculate_length(
        detect_points_line_up[0],
        detect_points_line_up[
            len(detect_points_line_up) - 1]) / mm_in_pixel

    thickness_down = calculate_length(
        detect_points_line_down[0],
        detect_points_line_down[
            len(detect_points_line_down) - 1]) / mm_in_pixel

    stem_thickness = thickness_up

    if thickness_up > thickness_down:
        stem_thickness = thickness_down

    return stem_thickness + 0.5


def calc_stems_thickness(image: np.ndarray,
                         size_qr_mm: float) -> List:
    """

    :param image:
    :param size_qr_mm:
    :return:
    """

    qrs_data = qr_code_detect(image)

    data = [inf.data for inf in qrs_data]

    result = [get_stem_thickness(image, inf.rect[2],
                                 inf.polygon, size_qr_mm) for inf in qrs_data]

    return list(zip(result, data))
