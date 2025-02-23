.. _softdevice_controller:

SoftDevice Controller
###########################

The |controller| is an RTOS-agnostic library built for the Nordic Semiconductor nRF52 and nRF53 Series.
It supports Bluetooth 5.
The library utilizes services provided by the :ref:`mpsl`.

The |controller| is distributed in different variants containing different features.
Variants for the Arm Cortex-M4 processor are available as soft-float, softfp-float, and hard-float.
Variants for the Arm Cortex-M33 processor are available as soft-float only.

|BLE| feature support:

+--------------------------+-----------------+--------------+-----------+
|                          | Peripheral-only | Central-only | Multirole |
+==========================+=================+==============+===========+
| 2 Mbps PHY               | X               | X            | X         |
+--------------------------+-----------------+--------------+-----------+
| Advertiser               | X               |              | X         |
+--------------------------+-----------------+--------------+-----------+
| Slave                    | X               |              | X         |
+--------------------------+-----------------+--------------+-----------+
| Scanner                  |                 | X            | X         |
+--------------------------+-----------------+--------------+-----------+
| Master                   |                 | X            | X         |
+--------------------------+-----------------+--------------+-----------+
| Data Length Extensions   | X               | X            | X         |
+--------------------------+-----------------+--------------+-----------+
| Advertising Extensions   |                 |              | X         |
+--------------------------+-----------------+--------------+-----------+
| Periodic Advertising     |                 |              | X         |
+--------------------------+-----------------+--------------+-----------+
| Coded PHY (Long Range)   |                 |              | X         |
+--------------------------+-----------------+--------------+-----------+

Proprietary feature support:

+--------------------------+-----------------+--------------+-----------+-----------------------------------------------------------------------------+
|                          | Peripheral-only | Central-only | Multirole | Description                                                                 |
+==========================+=================+==============+===========+=============================================================================+
| Low Latency Packet mode  |                 |              | X         | Enables using connection intervals below 7.5 ms                             |
+--------------------------+-----------------+--------------+-----------+-----------------------------------------------------------------------------+
| QoS Conn Event Reports   | X               | X            | X         | Reports QoS for every connection event.                                     |
|                          |                 |              |           | The application can then set an adapted channel map to avoid busy channels. |
+--------------------------+-----------------+--------------+-----------+-----------------------------------------------------------------------------+


.. note::

   Low Latency Packet mode is not supported on the nRF53 Series.

.. note::

   Periodic Advertising is supported as an experimental feature only.

.. toctree::
   :maxdepth: 2
   :caption: Subpages:

   doc/softdevice_controller
   doc/scheduling
   limitations
   CHANGELOG
   doc/api
