from tfrecords_utils import *
from image_augumentation import *
from segmentation_models import Unet, Linknet, FPN, PSPNet
from segmentation_models.backbones import get_preprocessing
from segmentation_models.losses import bce_jaccard_loss
from segmentation_models.metrics import iou_score
import argparse
import json
import datetime
from sklearn.model_selection import train_test_split


def main():

    parser = argparse.ArgumentParser(description='Train neural network model.')
    parser.add_argument('-itr', '--input-train', help='Path to the train TFRecord file.', type=str, required=True)
    parser.add_argument('-itv', '--input-validation', help='Path to the validation TFRecord file.', type=str,
                        default=None)
    parser.add_argument('-mn', '--model-name', help='Name of the model we want to initiate.', type=str, default='Unet')
    parser.add_argument('-sm', '--saved-model_path', help='Path to the saved model folder.', type=str, default=None)
    parser.add_argument('-fw', '--freeze-weights', help='Indicator if we want to keep backbone weights frozen.',
                        type=bool, default=True)
    parser.add_argument('-ep', '--epochs', help='Number of epochs for training.', type=int, default=100)
    parser.add_argument('-p', '--pretrained', help='Indicator if we want to do transfer learning.', type=bool,
                        default=True)
    parser.add_argument('-vs', '--validation-size', help='If input_validation is default, then use this size to split '
                                                         'training data into train/val.', type=float, default=0.2)
    parser.add_argument('-ac', '--augmentation-configuration', help='Augmentation dictionary with configuration.',
                        type=str, default=None)
    parser.add_argument('-al', '--augmentation-limit', help='Limit indicating how many images we want to have after '
                                                            'augmentation.', type=int, default=1000)
    options = parser.parse_args()

    # Extract arguments from argparse.
    input_validation = options.input_validation
    model_name = options.model_name
    saved_model_directory_path = options.saved_model_path
    freeze_weights = options.freeze_weights
    augmentation_configuration = options.augmentation_configuration

    if augmentation_configuration:

        # Load configuration file to a dictionary.
        with open(augmentation_configuration) as file:
            configuration_dictionary = json.load(file)
    else:
        configuration_dictionary = DEFAULT_AUGMENTOR_CONFIG

    # For now we set same backbone for all models, later argparse may take dictionary with parameters to initiate model.
    backbone = 'resnet34'
    preprocess_input = get_preprocessing(backbone)

    # Number of classes is set for 5 (later can be changed): fruit, stem, flower, leaf and background.
    number_of_classes = 5

    model_arguments = {
        'backbone_name': backbone,
        'activation': 'softmax',
        'classes': number_of_classes
    }

    if options.pretrained:
        model_arguments['encoder_weights'] = 'imagenet'
    else:
        freeze_weights = False

    if freeze_weights:
        model_arguments['encoder_freeze'] = True

    # Create the model.
    model = None
    if model_name == 'Unet':
        model = Unet(**model_arguments)
    elif model_name == 'Linknet':
        model = Linknet(**model_arguments)
    elif model_name == 'FPN':
        model = FPN(**model_arguments)
    elif model_name == 'PSPNet':
        model = PSPNet(**model_arguments)
    else:
        print("Unsupported model name. Currently supported models are: Unet, Linknet, FPN, PSPNet.")
        pass

    # Read TFRecord file.
    loaded_train_tfrecord_file = read_image_annotation_pairs_from_tfrecord(options.input_train)

    # Normalize images and masks.
    x_train, y_train = unpack_datasets(loaded_train_tfrecord_file)
    if input_validation:
        loaded_validation_tfrecord_file = read_image_annotation_pairs_from_tfrecord(input_validation)
        x_validation, y_validation = unpack_datasets(loaded_validation_tfrecord_file)
    else:
        x_train, x_validation, y_train, y_validation = train_test_split(
            x_train, y_train,
            test_size=options.validation_size,
            random_state=42
        )
    # Reshape images to fit backbone model (sometimes seem that this function is not working, may have to create custom
    # one)

    x_train, y_train = create_augmented_dataset(x_train, y_train, configuration=configuration_dictionary,
                                                number_of_examples=options.augmentation_limit)
    x_train = np.array(preprocess_input(x_train)) / 255
    y_train = np.array(preprocess_input(y_train)) / 255
    x_validation = np.array(preprocess_input(x_validation)) / 255
    y_validation = np.array(preprocess_input(y_validation)) / 255

    # Add callbacks.
    checkpoints_directory_path = 'checkpoints/cp-{epoch:04d}.ckpt'

    cp_callback = tf.keras.callbacks.ModelCheckpoint(
        checkpoints_directory_path,
        verbose=1,
        save_weights_only=True,
        period=5
    )

    # Add logging of training metrics.
    training_date = datetime.datetime.now()
    processed_date = training_date.strftime("%Y_%m_%d_%H")

    # Save training history into csv file. When append=False, then file will be overwritten with each script call.
    name = f'history_{processed_date}'
    csv_filename = f'training_metrics/{name}.csv'
    csv_logger = tf.keras.callbacks.CSVLogger(csv_filename, separator=',', append=False)

    model.save_weights(checkpoints_directory_path.format(epoch=0))

    # Compile the model.
    model.compile('Adam', loss=bce_jaccard_loss, metrics=[iou_score])

    # Train the model.
    model.fit(
        x=np.array(x_train),
        y=np.array(y_train),
        batch_size=16,
        epochs=100,
        callbacks=[cp_callback, csv_logger],
        validation_data=(np.array(x_validation), np.array(y_validation))
    )

    # Save model if passed in command line.
    if saved_model_directory_path:
        model.save(os.path.join(saved_model_directory_path, 'saved_model.h5'))


if __name__ == '__main__':
    main()
