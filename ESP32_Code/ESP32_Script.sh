#!/bin/bash

# ----- CHECKING IF ADDRESS FOR DATAFILE EXISTS -----
ADDR_DATAFILE="/home/pablo/GitHub/PROJ515_Dummy_Repo/ESP32_Arduino_Code/ESP32_Code/circ.csv"

# CHECK THROUGH PuTTY.
COM_PORT="/dev/rfcomm0"

# MAC ADDRESS OF BLUETOOTH DEVICE
MAC_ADDR="30:C6:F7:28:8A:EA"

if ! test -f "$ADDR_DATAFILE"
then
    echo "Invalid DataFile Link."
    exit
# ---------------------------------------------------
else
    rfcomm connect /dev/rfcomm0 $MAC_ADDR 1 &
    sleep 15
    # ----- CHECKING IF PORT IS OPERATIONAL ---------
    if ! command ls /dev/rfcomm* | grep -q "$COM_PORT"
    then
        echo "Not Available Port."
        exit
    # -----------------------------------------------
    else
        stty -F "$COM_PORT" 115200 cs8 -cstopb -parenb -ixon -ixoff -ignpar -ignbrk -inlcr -igncr -icrnl
        exec < "$COM_PORT"
        while true
        do
            # ----- CHECKING IF PORT IS ACTIVE ------
            # May need improving, to show that communication is actually happening.
            if read -r line
            then
            	if grep -q '1' <<< $line
            	then
            		echo "OK." > "$COM_PORT"
                       data=$(cat "$ADDR_DATAFILE")
                       echo "Commencing Transmission...." > "$COM_PORT"
                       echo "$data" > "$COM_PORT"
                       echo "Success!"
                else
                	echo "Wrong command" > "$COM_PORT"
                fi
            fi
            # --------------------------------------
        done
        exec <&-
    fi
fi
