.. _tfm_psa_level_1:

TF-M PSA Level 1
################

Overview
********
This TF-M integration example can be used with an Arm v8-M supported board. It
demonstrates how to use certain TF-M features that are covered as part of the
RTOS vendor requirements for a `PSA Certified Level 1`_ product, such as
secure storage for config data, initial attestation for device verification,
and the PSA crypto API for cryptography.

Trusted Firmware (TF-M) Platform Security Architecture (PSA) APIs
are used for the secure processing environment, with Zephyr running in the
non-secure processing environment.

As part of the standard build process, the secure bootloader (BL2) is built, as
well as the secure TF-M binary and non-secure Zephyr binary images. The two
application images are then merged and signed using the public/private key pair
stored in the secure bootloader, so that they will be accepted during the
image validation process at startup.

All TF-M dependencies will be automatically cloned in the ``/ext`` folder, and
built if appropriate build artefacts are not detected.

.. _PSA Certified Level 1:
  https://www.psacertified.org/security-certification/psa-certified-level-1/

.. note:: This example is based upon trusted-firmware-m 1.0-rc2 (commit 4117e0353c55dce739f550515c5b007411142ba7)

Building and Running
********************

This project outputs startup status and info to the console. It can be built and
executed on an MPS2+ configured for AN521 (dual-core ARM Cortex M33).

This sample will only build on a Linux or macOS development system
(not Windows), and has been tested on the following setups:

- macOS Mojave using QEMU 4.1.0 with gcc-arm-none-eabi-7-2018-q2-update
- macOS Mojave with gcc-arm-none-eabi-7-2018-q2-update
- Ubuntu 18.04 using Zephyr SDK 0.10.1

On MPS2+ AN521:
===============

1. Build Zephyr with a non-secure configuration
   (``-DBOARD=mps2_an521_nonsecure``). This will also build TF-M in the
   ``ext/tfm`` folder if necessary.

Using ``west``

.. code-block:: bash

   cd <ZEPHYR_ROOT>/samples/tfm_integration/psa_level_1/
   west build -p -b mps2_an521_nonsecure ./

Using ``cmake``

.. code-block:: bash

   cd <ZEPHYR_ROOT>/samples/tfm_integration/psa_level_1/
   mkdir build
   cd build
   cmake -GNinja -DBOARD=mps2_an521_nonsecure ..
   ninja -v

2. Copy application binary files (mcuboot.bin and tfm_sign.bin) to
   ``<MPS2 device name>/SOFTWARE/``.

3. Edit (e.g., with vim) the ``<MPS2 device name>/MB/HBI0263C/AN521/images.txt``
   file, and update it as shown below:

   .. code-block:: bash

      TITLE: Versatile Express Images Configuration File

      [IMAGES]
      TOTALIMAGES: 2 ;Number of Images (Max: 32)

      IMAGE0ADDRESS: 0x10000000
      IMAGE0FILE: \SOFTWARE\mcuboot.bin  ; BL2 bootloader

      IMAGE1ADDRESS: 0x10080000
      IMAGE1FILE: \SOFTWARE\tfm_sign.bin ; TF-M with application binary blob

4. Save the file, exit the editor, and reset the MPS2+ board.

On QEMU:
========

1. Build Zephyr with a non-secure configuration
   (``-DBOARD=mps2_an521_nonsecure``). This will also build TF-M in the
   ``ext/tfm`` folder if necessary.

Using ``west``

.. code-block:: bash

   cd <ZEPHYR_ROOT>/samples/tfm_integration/psa_level_1/
   west build -p -b mps2_an521_nonsecure ./

Using ``cmake``

.. code-block:: bash

   cd <ZEPHYR_ROOT>/samples/tfm_integration/psa_level_1/
   mkdir build
   cd build
   cmake -GNinja -DBOARD=mps2_an521_nonsecure ..
   ninja -v

2. Run the qemu startup script, which will merge the key binaries and start
   execution of QEMU using the AN521 build target:

.. code-block:: bash

   ./qemu.sh

Sample Output
=============

.. code-block:: console

   [INF] Starting bootloader
   [INF] Swap type: none
   [INF] Bootloader chainload address offset: 0x80000
   [INF] Jumping to the first image slot
   [Sec Thread] Secure image initializing!
   TFM level is: 1
   [Sec Thread] Jumping to non-secure code...
   ***** Booting Zephyr OS build zephyr-v1.14.0-2726-g611526e98102 *****
   [00:00:00.000,000] <inf> app: app_cfg: Creating new config file with UID 0x155cfda7a
   [00:00:00.010,000] <inf> app: att: System IAT size is: 495 bytes.
   [00:00:00.010,000] <inf> app: att: Requesting IAT with 64 byte challenge.
   [00:00:00.100,000] <inf> app: att: IAT data received: 495 bytes.
             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
   00000000 D2 84 43 A1 01 26 A1 04 58 20 07 8C 18 F1 10 F4 ..C..&..X ......
   00000010 32 FF 78 0C D8 DA E5 80 69 A2 A0 D8 22 77 CB C6 2.x.....i..."w..
   00000020 64 50 C8 58 1D D4 7D 96 A2 2E 59 01 80 AA 3A 00 dP.X..}...Y...:.
   00000030 01 24 FF 58 40 00 11 22 33 44 55 66 77 88 99 AA .$.X@.."3DUfw...
   00000040 BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99 AA ......."3DUfw...
   00000050 BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99 AA ......."3DUfw...
   00000060 BB CC DD EE FF 00 11 22 33 44 55 66 77 88 99 AA ......."3DUfw...
   00000070 BB CC DD EE FF 3A 00 01 24 FB 58 20 A0 A1 A2 A3 .....:..$.X ....
   00000080 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0 B1 B2 B3 ................
   00000090 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF 3A 00 01 25 ............:..%
   000000A0 01 77 77 77 77 2E 74 72 75 73 74 65 64 66 69 72 .wwww.trustedfir
   000000B0 6D 77 61 72 65 2E 6F 72 67 3A 00 01 24 F7 71 50 mware.org:..$.qP
   000000C0 53 41 5F 49 4F 54 5F 50 52 4F 46 49 4C 45 5F 31 SA_IOT_PROFILE_1
   000000D0 3A 00 01 25 00 58 21 01 FA 58 75 5F 65 86 27 CE :..%.X!..Xu_e.'.
   000000E0 54 60 F2 9B 75 29 67 13 24 8C AE 7A D9 E2 98 4B T`..u)g.$..z...K
   000000F0 90 28 0E FC BC B5 02 48 3A 00 01 24 FC 72 30 36 .(.....H:..$.r06
   00000100 30 34 35 36 35 32 37 32 38 32 39 31 30 30 31 30 0456527282910010
   00000110 3A 00 01 24 FA 58 20 AA AA AA AA AA AA AA AA BB :..$.X .........
   00000120 BB BB BB BB BB BB BB CC CC CC CC CC CC CC CC DD ................
   00000130 DD DD DD DD DD DD DD 3A 00 01 24 F8 20 3A 00 01 .......:..$. :..
   00000140 24 F9 19 30 00 3A 00 01 24 FD 81 A6 01 68 4E 53 $..0.:..$....hNS
   00000150 50 45 5F 53 50 45 04 65 30 2E 30 2E 30 03 00 02 PE_SPE.e0.0.0...
   00000160 58 20 52 ED 0E 2C F2 D2 D2 36 E0 CF 76 FD C2 64 X R..,...6..v..d
   00000170 1F E0 28 2E AA EF 14 A7 FB AE 92 52 C0 D1 5F 61 ..(........R.._a
   00000180 81 8A 06 66 53 48 41 32 35 36 05 58 20 BF E6 D8 ...fSHA256.X ...
   00000190 6F 88 26 F4 FF 97 FB 96 C4 E6 FB C4 99 3E 46 19 o.&..........>F.
   000001A0 FC 56 5D A2 6A DF 34 C3 29 48 9A DC 38 58 40 D9 .V].j.4.)H..8X@.
   000001B0 49 32 21 DB 84 16 89 A7 43 33 E4 9C DF EF 55 07 I2!.....C3....U.
   000001C0 C2 81 85 C7 AE 54 77 D9 A1 66 6A B0 76 77 7A 0E .....Tw..fj.vwz.
   000001D0 15 08 49 13 B5 2D CC C8 53 EC D0 01 40 C2 63 84 ..I..-..S...@.c.
   000001E0 A4 70 68 71 0A 71 BB BC 37 43 CD E5 0B DB A4    .phq.q..7C.....
   [00:00:00.098,000] <inf> app: Generating 256 bytes of random data.
             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
   00000000 F5 AA 00 93 43 C2 23 7D AE 99 75 4B AB 65 E6 68 ....C.#}..uK.e.h
   00000010 CA 15 B0 D0 5A EA 17 5C EC 46 D4 4B 5D 8D AF 9F ....Z..\.F.K]...
   00000020 20 2E A6 B5 7D 8E 63 4D A2 97 20 04 DF 73 F3 20  ...}.cM.. ..s.
   00000030 E4 6F 72 A3 57 59 EA 1F AD 04 A7 B0 BA 71 9A 2C .or.WY.......q.,
   00000040 D5 9A 34 26 76 DC EA 5D EE 02 EB 1C 68 3A C4 E9 ..4&v..]....h:..
   00000050 27 A0 31 8B 0A B1 02 E2 D3 57 7A 3D 33 27 74 94 '.1......Wz=3't.
   00000060 7B BC B8 89 33 81 05 06 F3 B4 01 B5 F9 31 B9 6D {...3........1.m
   00000070 D8 D7 74 A8 58 2F E4 25 25 58 C6 60 C0 83 65 73 ..t.X/.%%X.`..es
   00000080 F0 EA 8A 30 94 1F AB A2 29 14 3D B4 B0 50 6B 6F ...0....).=..Pko
   00000090 7D EC 91 4D A6 41 DD 99 AF 22 2C 1C E1 91 29 8A }..M.A...",...).
   000000A0 E5 B7 51 33 44 83 0E F9 0A B3 AC EE CD DC 17 47 ..Q3D..........G
   000000B0 B9 91 D5 72 B4 96 FD D6 F8 72 6C D6 B1 A5 C2 D7 ...r.....rl.....
   000000C0 3F 40 DE 72 0D 2C A3 7B 58 E2 0D B1 CF C8 31 A0 ?@.r.,.{X.....1.
   000000D0 C1 AD 1F 36 C6 F7 4C B0 2A 76 6E 83 5D F3 FD 0C ...6..L.*vn.]...
   000000E0 6B 97 5C E4 10 18 60 6E A2 AC DA 11 70 E1 85 5B k.\...`n....p..[
   000000F0 1C EB BF 0E 48 11 0E DE FA E2 4A 0E DE 1C 42 A0 ....H.....J...B.
   [00:00:00.102,000] <inf> app: Calculating SHA-256 hash of value.
             0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
   00000000 6B 22 09 2A 37 1E F5 14 F7 39 4D CF AD 4D 17 46
   00000010 66 CB 33 A0 39 D8 41 4E F1 2A D3 4D 69 C3 B5 3E

Signing Images
==============

TF-M uses a secure bootloader (BL2) and firmware images must be signed
with a private key before execution can be handed off by the bootloader. The
firmware image is validated by the bootloader at startup using the public key,
which is built into the secure bootloader.

By default, ``tfm/bl2/ext/mcuboot/root-rsa-3072.pem`` is used to sign images.
``merge.sh`` signs the TF-M + Zephyr binary using the .pem private key,
calling ``imgtool.py`` to perform the actual signing operation.

To satisfy PSA Level 1 certification requirements, **You MUST replace
the default .pem file with a new key pair!**

To generate a new public/private key pair, run the following commands from
the sample folder:

.. code-block:: bash

  $ chmod +x ../../../ext/tfm/tfm/bl2/ext/mcuboot/scripts/imgtool.py
  $ ../../../ext/tfm/tfm/bl2/ext/mcuboot/scripts/imgtool.py keygen \
    -k root-rsa-3072.pem -t rsa-3072

You can then replace the .pem file in ``/ext/tfm/tfm/bl2/ext/mcuboot/`` with
the newly generated .pem file, and rebuild the bootloader so that it uses the
public key extracted from this new key file when validating firmware images.

.. code-block:: bash

  $ west build -p -b mps2_an521_nonsecure ./
  $ ./merge.sh

.. warning::

  Be sure to keep your private key file in a safe, reliable location! If you
  lose this key file, you will be unable to sign any future firmware images,
  and it will no longer be possible to update your devices in the field!
