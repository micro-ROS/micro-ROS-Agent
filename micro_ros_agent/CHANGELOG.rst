^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package micro-ros_agent
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3.0.6 (2024-01-29)
------------------
* Fix thread include (backport `#216 <https://github.com/micro-ROS/micro-ROS-Agent/issues/216>`_) (`#217 <https://github.com/micro-ROS/micro-ROS-Agent/issues/217>`_)

3.0.5 (2023-06-06)
------------------

3.0.4 (2022-09-28)
------------------
* Fix Datawriter destruction (`#169 <https://github.com/micro-ROS/micro-ROS-Agent/issues/169>`_)
* Synchronise predicate (`#160 <https://github.com/micro-ROS/micro-ROS-Agent/issues/160>`_)

3.0.3 (2022-06-13)
------------------
* Fix memory leak in graph manager (`#147 <https://github.com/micro-ROS/micro-ROS-Agent/issues/147>`_)

3.0.2 (2022-05-25)
------------------

3.0.1 (2022-03-25)
------------------
* Add services to graph manager (`#127 <https://github.com/micro-ROS/micro-ROS-Agent/issues/127>`_) (`#129 <https://github.com/micro-ROS/micro-ROS-Agent/issues/129>`_)
* Add used missing includes (`#116 <https://github.com/micro-ROS/micro-ROS-Agent/issues/116>`_) (`#124 <https://github.com/micro-ROS/micro-ROS-Agent/issues/124>`_)
* Add system logger flag (`#118 <https://github.com/micro-ROS/micro-ROS-Agent/issues/118>`_) (`#119 <https://github.com/micro-ROS/micro-ROS-Agent/issues/119>`_)
* Add condition variable include (`#113 <https://github.com/micro-ROS/micro-ROS-Agent/issues/113>`_) (`#114 <https://github.com/micro-ROS/micro-ROS-Agent/issues/114>`_)
* pass system name to xrceagent (`#110 <https://github.com/micro-ROS/micro-ROS-Agent/issues/110>`_) (`#112 <https://github.com/micro-ROS/micro-ROS-Agent/issues/112>`_)
* Fix memory leak in FastDDS datawriter (`#107 <https://github.com/micro-ROS/micro-ROS-Agent/issues/107>`_) (`#109 <https://github.com/micro-ROS/micro-ROS-Agent/issues/109>`_)

3.0.0 (2021-09-13)
------------------

1.0.1 (2021-09-13)
------------------
* Remove XRCE dependency and add superbuild (`#97 <https://github.com/micro-ROS/micro-ROS-Agent/issues/97>`_)
* Fixed launch file by using a list for arguments. Ensures order of items is kept. (`#93 <https://github.com/micro-ROS/micro-ROS-Agent/issues/93>`_) (`#94 <https://github.com/micro-ROS/micro-ROS-Agent/issues/94>`_)
* Modify argument type (`#91 <https://github.com/micro-ROS/micro-ROS-Agent/issues/91>`_)
* Fix graph manager datawriters behaviour (`#84 <https://github.com/micro-ROS/micro-ROS-Agent/issues/84>`_)
* Graph manager: Fix participant mask for listener callbacks (`#81 <https://github.com/micro-ROS/micro-ROS-Agent/issues/81>`_)
* Fix agent launch (`#78 <https://github.com/micro-ROS/micro-ROS-Agent/issues/78>`_)
* Fix graph manager node namespaces (`#75 <https://github.com/micro-ROS/micro-ROS-Agent/issues/75>`_)
* Fix Rolling agent (`#61 <https://github.com/micro-ROS/micro-ROS-Agent/issues/61>`_)
* Add multi domain graph manager (`#69 <https://github.com/micro-ROS/micro-ROS-Agent/issues/69>`_)
* Add ros2 launch capabilities and example launch file (`#47 <https://github.com/micro-ROS/micro-ROS-Agent/issues/47>`_)
* Snap build for the micro-ROS-Agent (`#43 <https://github.com/micro-ROS/micro-ROS-Agent/issues/43>`_)
* Create graph manager after checking passed CLI arguments and launch xrce-dds server (`#41 <https://github.com/micro-ROS/micro-ROS-Agent/issues/41>`_)
* Contributors: Antonio Cuadros, Jose Antonio Moral, Pablo Garrido, mergify[bot]

0.0.1 (2019-04-24)
-----------------
* Initial release

