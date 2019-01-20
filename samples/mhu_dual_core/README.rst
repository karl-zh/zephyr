.. _mhu_dual_core:

MHU Dual Core
###########

Overview
********
A simple dual core example that can be used with arm Musca A1
prints 'Hello World' to the console from each core. 
This application can be built into modes:

* single thread

Building and Running
********************

This project outputs 'Hello World' to the console.  It can be built and executed
on Musca A1 as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/hello_world
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

To build the single thread version, use the supplied configuration file for
single thread: :file:`prj_single.conf`:

.. zephyr-app-commands::
   :zephyr-app: samples/hello_world
   :host-os: unix
   :board: qemu_x86
   :conf: prj_single.conf
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

    Hello World! x86
