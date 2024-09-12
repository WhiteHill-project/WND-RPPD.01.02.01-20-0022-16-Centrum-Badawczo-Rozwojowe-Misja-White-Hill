import argparse
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from os.path import basename
from typing import List, Dict, Tuple
import random
import datetime
import os


def get_all_columns(list_of_dataframes: List[pd.DataFrame]) -> List[str]:
    """
    Function to get all unique column names from multiple dataframes.

    :param list_of_dataframes: List of dataframes from which we want to take column names.
    :return: List of unique column names from all dataframes provided in list_of_dataframes.
    """

    columns = []

    # Get column names from each dataframe.
    for dataframe in list_of_dataframes:
        columns.append(dataframe.columns.values)

    # Flatten the list of column names lists and get return unique ones.
    flat_columns = [col for column in columns for col in column]

    return list(set(flat_columns))


def get_colors_for_files(list_of_files: List[str]) -> Dict:
    """
    Function to assign random color to a file from input list. Created to keep truck of colors using
    matplotlib.subplots.

    :param list_of_files: List of file names.
    :return: Dictionary with file names as keys and colors signatures as values.
    """

    # List signatures of possible colors used in matplotlib plotting.
    possible_colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
    colors = possible_colors.copy

    # Iterate over the file names and assign colors to them randomly. For easier usage each file will have different
    # color unless there are more than 7 files.
    file_to_color_dictionary = {}
    for file in list_of_files:

        # For 8th file and after we have to repeat the colors.
        if len(possible_colors) == 0:
            possible_colors = colors

        # Assign random color from list to a file.
        file_color = random.choice(possible_colors)
        file_to_color_dictionary[file] = file_color

        # Remove that color from list. We are doing that to avoid unnecessary color repeats.
        possible_colors.remove(file_color)

    return file_to_color_dictionary


def pair_train_validation_metrics(dataframe: pd.DataFrame) -> List[Tuple[str, str]]:
    """
    Function to get list of pair of (metric, val_metric) for each metric in dataframe.

    :param dataframe: Pandas DataFrame containing training history log.
    :return: List of (metric, val_metric) tuples for given training history log.
    """

    # Get all columns, each column name represents a metric from logs.
    columns = dataframe.columns.values

    # Iterate over column names and get pairs if metric exists both for training and validation.
    pairs = []
    for column in columns:
        if 'val' not in column:
            if f'val_{column}' in columns:
                pairs.append((column, f'val_{column}'))

    return pairs


def get_name_metric_pairs(dataframes_dictionary: Dict[str, pd.DataFrame]) -> List[Tuple[str, str]]:
    """
    Function to get (file_name, metric) tuples for all files and all metrics for each one.

    :param dataframes_dictionary: Dictionary in which keys are file names and values are dataframes.
    :return: List of (file_name, metric) tuples.
    """

    # Iterate over items in dataframes_dictionary and create (file_name, metric) pairs.
    file_metric = []
    for file_name, dataframe in dataframes_dictionary.items():
        pairs = pair_train_validation_metrics(dataframe)
        metrics = [metric for metric, _ in pairs]
        for metric in metrics:
            file_metric.append((file_name, metric))

    return file_metric


def calculate_number_of_subplots(list_of_dataframes: List[pd.DataFrame]) -> int:
    """
    Function to calculate number of needed subplots for inside metric comparing.

    :param list_of_dataframes: List of dataframes.
    :return: Number of needed subplots for dataframes in input list.
    """

    # Create and count train_validation_metrics pairs.
    number_of_subplots = 0
    for dataframe in list_of_dataframes:
        number_of_subplots += len(pair_train_validation_metrics(dataframe))

    return number_of_subplots


def compare_inside(history_dataframes_dictionary: Dict, display: bool, plot_directory_path: str, processed_date: str):
    """
    Function to create report for each metric for each log (i.e. for each log we have train and validation metric
    compared on the same graph).

    :param history_dataframes_dictionary: Dictionary containing logs in form of pd.DataFrame's.
    :param display: Boolean indicator if we want to plot the graphs.
    :param plot_directory_path: Path to the directory in which we want to save the plots.
    :param processed_date: Processed date used in plot file name.
    """

    fig_inside, ax = plt.subplots(calculate_number_of_subplots(list(history_dataframes_dictionary.values())), 1,
                                  figsize=(12, 12))

    for i, file_metric_pair in enumerate(get_name_metric_pairs(history_dataframes_dictionary)):

        # Unpack (file_name, metric) pair.
        file_name, metric = file_metric_pair

        # Set x-axis values to integers.
        ax[i].xaxis.set_major_locator(MaxNLocator(integer=True))

        # Add title for each subplot as metric presented on said subplot.
        ax[i].title.set_text(f'{file_name} - {metric}')

        # Get metric values for training and validation.
        dataframe = history_dataframes_dictionary[file_name]
        metric_training_values = dataframe[metric].tolist()
        metric_validation_values = dataframe[f'val_{metric}']

        # Plot metrics.
        ax[i].plot(metric_training_values, 'k', label=metric)
        ax[i].plot(metric_validation_values, 'm', label=f'val_{metric}')

        # Shrink subplots a little and place legend places left center outside plot.
        box = ax[i].get_position()
        ax[i].set_position([box.x0, box.y0, box.width * 0.8, box.height])
        ax[i].legend(loc='center left', bbox_to_anchor=(1, 0.5))

    if display:
        plt.show()

    if plot_directory_path:
        fig_inside.savefig(os.path.join(plot_directory_path, f'compare_inside_{processed_date}.jpg'))


def compare_between(unique_column_names: List[str], history_dataframes_dictionary: Dict, file_colors_dictionary: Dict,
                    display: bool, plot_directory_path: str, processed_date: str):
    """
    Function to create report for each metric for each log (i.e. for each metric we have each log compared on the same
    graph).

    :param unique_column_names: List of unique column names.
    :param history_dataframes_dictionary: Dictionary containing logs in form of pd.DataFrame's.
    :param file_colors_dictionary: Dictionary containing information about colors.
    :param display: Boolean indicator if we want to plot the graphs.
    :param plot_directory_path: Path to the directory in which we want to save the plots.
    :param processed_date: Processed date used in plot file name.
    """

    fig_between, ax = plt.subplots(len(unique_column_names), 1, figsize=(12, 12))

    # Iterate over each unique metric.
    for i, column_name in enumerate(unique_column_names):

        # Set x-axis values to integers.
        ax[i].xaxis.set_major_locator(MaxNLocator(integer=True))

        # Add title for each subplot as metric presented on said subplot.
        ax[i].title.set_text(column_name)

        # Iterate over the items in history_dataframes_dictionary.
        for file_name, dataframe in history_dataframes_dictionary.items():

            # Check if metric exist in dataframe. It might not exist if new metric was logged after some of logs
            # were already created.
            if column_name in dataframe.columns:
                column_values = dataframe[column_name].tolist()

                # Plot metric.
                ax[i].plot(column_values, file_colors_dictionary[file_name], label=file_name)

                # Shrink subplots a little and place legend places left center outside plot.
                box = ax[i].get_position()
                ax[i].set_position([box.x0, box.y0, box.width * 0.8, box.height])
                ax[i].legend(loc='center left', bbox_to_anchor=(1, 0.5))

    if display:
        plt.show()

    if plot_directory_path:
        fig_between.savefig(os.path.join(plot_directory_path, f'compare_between_{processed_date}.jpg'))


def main():

    parser = argparse.ArgumentParser(description='Script to create model training report.')
    parser.add_argument('-hp', '--history-paths', help='Paths to the model history files we want to analyze/compare.',
                        nargs='+', required=True)
    parser.add_argument('-sd', '--save-directory', help='Optional path to the directory in which we want to save the '
                                                        'plots.', type=str, default=None)
    parser.add_argument('-ct', '--check-types', help='Report types: "compare_between" shows plots containing same '
                                                     'metric for multiple dataframes, "compare_inside" shows plots for '
                                                     'each dataframe containing training and validation values of the '
                                                     'same metric.',
                        nargs='+', default=['compare_between', 'compare_inside'])
    parser.add_argument('-d', '--display', help='Indicator if we want to display the plots.', type=bool, default=False)

    options = parser.parse_args()

    training_date = datetime.datetime.now()
    processed_date = training_date.strftime("%Y_%m_%d_%H")

    # Extract list of paths to the csv files with training history.
    list_of_history_files = options.history_paths
    plot_directory_path = options.save_directory
    check_types = options.check_types
    display = options.display

    # Create dictionary containing history csv files as keys and corresponding pandas DataFrames as values.
    history_dataframes_dictionary = {}
    for history_file in list_of_history_files:
        history_dataframes_dictionary[basename(history_file)] = pd.read_csv(history_file, index_col=0)

    # Get unique column names.
    unique_column_names = get_all_columns(list(history_dataframes_dictionary.values()))

    # Assign colors to each file.
    file_colors_dictionary = get_colors_for_files(list(history_dataframes_dictionary.keys()))

    if 'compare_between' in check_types:
        compare_between(
            unique_column_names=unique_column_names,
            history_dataframes_dictionary=history_dataframes_dictionary,
            file_colors_dictionary=file_colors_dictionary,
            display=display,
            plot_directory_path=plot_directory_path,
            processed_date=processed_date
        )

    if 'compare_inside' in check_types:
        compare_inside(
            history_dataframes_dictionary=history_dataframes_dictionary,
            display=display,
            plot_directory_path=plot_directory_path,
            processed_date=processed_date
        )


if __name__ == '__main__':
    main()
