# Subscribe
This sample will subscribe to a given signal and print any published values to this signal.

## Usage

As all python samples in this repository, the pip package [remotivelabs-broker](https://pypi.org/project/remotivelabs-broker/) is required. Install all requirements with [pip](https://pypi.org/):

    pip install -r requirements.txt

Subscribe to a signal by running.

    python subscribe.py --url http://192.168.4.1:50051  --signals ecu_A:TestFr06_Child02

This will subscribe to the signal `TestFr06_Child02` from the namespace `ecu_A` and the signal `TestFr06_Child04` 
from namespace `ecu_B`. 

    python subscribe.py --url http://192.168.4.1:50051 --signals ecu_A:TestFr06_Child02 ecu_B:TestFr06_Child04


If corresponding namespace and signal does not exist on the broker an error message will be shown.

    python subscribe.py --url http://192.168.4.1:50051  --signals ecu_A:NonExistingSignal

    One or more signals you subscribed to does not exist , {'NonExistingSignal'}


