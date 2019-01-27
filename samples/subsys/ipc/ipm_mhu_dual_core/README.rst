.. _ipm_mhu_dual_core:

MHU Dual Core
###########

Overview
********
A simple dual core example that can be used with arm Musca A1
prints 'IPM MHU sample ' to the console from each core.
This application can be built into secure and non_secure:

* single thread

Building and Running
********************

This project outputs 'IPM MHU sample on musca_a' to the console.
It can be built and executed on Musca A1 CPU 0 as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/ipc/ipm_mhu_dual_core
   :board: v2m_musca
   :goals: run
   :compact:

This project outputs 'IPM MHU sample on v2m_musca_nonsecure' to the console.
It can be built and executed on Musca A1 CPU 1 as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/ipc/ipm_mhu_dual_core
   :board: v2m_musca_nonsecure
   :goals: run
   :compact:

Sample Output
=============

.. code-block:: console

   *** Booting Zephyr OS zephyr-v1.13.0-3378-g3625524 ***
   IPM MHU sample on musca_a
   CPU 0, get MHU0 success!
   *** Booting Zephyr OS zephyr-v1.13.0-3378-g3625524 ***
   IPM MHU sample on musca_a_nonsecure
   CPU 1, get MHU0 success!
   MHU ISR on CPU 0
   MHU ISR on CPU 1
   MHU Test Done.
