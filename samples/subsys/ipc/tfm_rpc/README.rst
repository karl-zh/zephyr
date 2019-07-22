.. _TFM_rpc:

TFM RPC Application
###################

Overview
********

This application shows how to call secure service with Zephyr on another
core. It is designed to demonstrate how to integrate OpenAMP with remote
procedure call (RPC) in Zephyr as client. This sample run with TFM, so it will
depends on TFM's support with eRPC as server.

Building the application for mps2_an521_nonsecure
*************************************************

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/ipc/tfm_rpc
   :board: mps2_an521_nonsecure
   :goals: debug

Open a serial terminal (minicom, putty, etc.) and connect the board with the
following settings:

- Speed: 115200
- Data: 8 bits
- Parity: None
- Stop bits: 1

Reset the board and the following message will appear on the corresponding
serial port, one is master another is remote:

.. code-block:: console

   [Sec Thread] Secure image initializing!
   ***** Booting Zephyr OS zephyr-v1.14.0-1616-g3013993758d4 *****
   The version of the PSA Framework API is 256.
   The minor version is 1.
   Connect success!
   TFM service support minor version is 1.
   psa_call is successful!
   outvec1 is: It is just for IPC call test.
   outvec2 is: It is just for IPC call test.
   Hello World! mps2_an521_nonsecure
