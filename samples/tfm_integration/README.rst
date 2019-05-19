.. _tfm_integration:

TF-M Integration
################

Overview
********
A simple TF-M integration example that can be used with arm v8-M supported
board.
To run sample for TF-M in secure and Zephyr in non-secure, Zephyr requsets
services by PSA APIs to TF-M. It prints test info to the console. This
application can be built into modes:

* single thread
* multi threading

Building and Running
********************

This project outputs test status and info to the console. It can be built and executed
on MPS2+ AN521 as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/tfm_integration
   :host-os: unix
   :board: MPS2+ AN521
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

    [Sec Thread] Secure image initializing!
    [Sec Thread] hello! this is ipc client test sp!
    [Sec Thread] Connect success!
    [Sec Thread] Call success!
    **** Booting Zephyr OS zephyr-v1.14.0-512-gf26982cc00d3 ****
    The version of the PSA Framework API is 256.
    The minor version is 1.
    Connect success!
    TFM service support minor version is 1.
    psa_call is successful!
    outvec1 is: It is just for IPC call test.
    outvec2 is: It is just for IPC call test.
    Hello World! mps2_an521_nonsecure
