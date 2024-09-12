from keras.models import Sequential
from keras.layers.core import Activation, Reshape, Permute
from keras.layers.convolutional import Convolution2D, MaxPooling2D, UpSampling2D
from keras.layers.normalization import BatchNormalization
from keras.layers import Conv2D


def model(image_height: int, image_width: int, number_of_classes: int, kernel: int = 3, activation: str = 'relu'):

    encoding_layers = [
        Conv2D(64, (kernel, kernel), input_shape=(image_height, image_width, 3), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(64, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        MaxPooling2D(),

        Convolution2D(128, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(128, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        MaxPooling2D(),


        Convolution2D(256, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(256, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(256, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        MaxPooling2D(),

        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        MaxPooling2D(),

        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        MaxPooling2D()
    ]

    autoencoder = Sequential()
    autoencoder.encoding_layers = encoding_layers

    for layer in autoencoder.encoding_layers:
        autoencoder.add(layer)

    decoding_layers = [
        UpSampling2D(),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),

        UpSampling2D(),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(512, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(256, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),

        UpSampling2D(),
        Convolution2D(256, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(256, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(128, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),

        UpSampling2D(),
        Convolution2D(128, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(64, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),

        UpSampling2D(),
        Convolution2D(64, (kernel, kernel), padding='same'),
        BatchNormalization(),
        Activation(activation),
        Convolution2D(number_of_classes, (1, 1), padding='valid', activation="sigmoid"),
        BatchNormalization()
    ]

    autoencoder.decoding_layers = decoding_layers
    for layer in decoding_layers:
        autoencoder.add(layer)

    autoencoder.add(Reshape((number_of_classes, image_height * image_width)))
    autoencoder.add(Permute((2, 1)))
    autoencoder.add(Activation('softmax'))

    return autoencoder
