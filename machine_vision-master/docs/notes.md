<div align="center">
  <img src="https://whitehill.eu/wp-content/uploads/2017/10/Triffid-prop-logo-z-podpisem.png">
</div>

Table of contents
=================
<a name="table-of-contents">

* [Overview](#overview)
* [Notations](#notations)
    * [Train and test datasets](#train-and-test-datasets)
    * [Image name](#image-name)
    * [Panoramic image name](#panoramic-image-name)
    * [Database schema](#database-schema)
    

Overview
========
<a name="overview">

Fluid collection of notes and discussed topics.

Notations
=========
<a name="notations">


Different notations proposed for the **_TRIFFID_** project.

Train and test datasets
-----------------------
<a name="train-and-test-datasets">


Here is a proposed folder structure for keeping train and test datasets.

* train
    * train_image_1_name
        * images
            * train_image_1_name_original.jpeg
        * masks
            * train_image_1_name_mask.jpeg
    * train_image_2_name
        * images
            * train_image_2_name_original.jpeg
        * masks
            * train_image_2_name_mask.jpeg
    * ...
* test
    * test_image_1_name
        * images
            * test_image_1_name_original.jpeg
        * masks
            * test_image_1_name_mask.jpeg
    * test_image_2_name
        * images
            * test_image_2_name_original.jpeg
        * masks
            * test_image_2_name_mask.jpeg
    * ...

Image name
----------
<a name="image-name">


<p align="center">
    <b>[greenhouse_id]_[lane_id]_[sequence_id]_[side]_[date]_[image_type].jpeg</b>
</p>

where:

* greenhouse_id: id of the greenhouse in which we have taken the image
* lane_id: id of the row in which we have taken the image
* sequence_id: number in order of the image for given row
* side: which site the camera was facing when taking a image, possible values: 'left', 'right'
* date: date of the taking the image, [INFO] may be replaced by the id of the lap over greenhouse
if lap over whole structure is taken over a few days
* image_type: indicator if the image is the original or the mask

Some of those indicators might be moved to the folder structure, but they still can be useful to 
debug if images happened to be placed in the wrong folders.

Panoramic image name
--------------------
<a name="panoramic-image-name">

For 'panoramic' image naming convention should be similar:

<p align="center">
    <b>panoramic_[greenhouse_id]_[lane_id]_[side]_[date]_[image_type].jpeg</b>
</p>

We do not need to have [sequence_id] since our stitched image consists of all images in a sequence.
We add _panoramic_ flag to show it is stitched image.

Database schema
---------------
<a name='database-schema'>

For database we propose Postgres database. Below we can see suggested tables with specific columns
and data types:

1) Company
    * country: text
    * region: text
    * city: text
    * street: text
2) Greenhouse - this table contains data about all greenhouses within company.
    * name: text
3) Lane - this table contains data about all lanes within a greenhouse.
    * position: integer
    * length: numeric
    * side: text
4) Lap - each record in this table corresponds with one lap around the greenhouse (full lap i.e. TRIFFID gather data 
from all lanes).
    * start_timestamp: timestamp without time zone
    * end_timestamp: timestamp without time zone
5) Readout - each record in this table corresponds to a point in a greenhouse at which measurements were taken.
    * timestamp: timestamp without time zone
    * image_path: text
    * mask_path: text
6) Markers - each record of this table contains markers and indicators calculated from specific image.
    * number_of_fruits: integer
    * number_of_flowers: integer
    * green_surface_area: numeric
    * fruit_surface_area: numeric
    * stalk_thickness: numeric
    * fruit_weight: numeric
    * harvest_index: numeric
    
List of tables and columns for this database may extend with time, depending on needs.