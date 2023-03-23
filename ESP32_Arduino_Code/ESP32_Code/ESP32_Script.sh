#!/bin/bash

# ----- CHECKING IF ADDRESS FOR DATAFILE EXISTS -----
ADDR_DATAFILE="C:/Users/Christopher Jenner/Documents/GitHub/PROJ515_Dummy_Repo/ESP32_Arduino_Code/ESP32_Code/data.csv"

# CHECK THROUGH PuTTY.
COM_PORT="/dev/ttyUSB0"

if ! test -f "$ADDR_DATAFILE"; then
    echo "Invalid DataFile Link."
    exit
# ---------------------------------------------------
else
    # ----- CHECKING IF PORT IS OPERATIONAL ---------
    if ! command ls /dev/tty* | grep -q "$COM_PORT"; then
        echo "Not Available Port."
        exit
    # -----------------------------------------------
    else
        stty -F "$COM_PORT" 115200 cs8 -cstopb -parenb -ixon -ixoff -ignpar -ignbrk -inlcr -igncr -icrnl
        exec < "$COM_PORT"
        while true; do
            # ----- CHECKING IF PORT IS ACTIVE ------
            # May need improving, to show that communication is actually happening.
            if test -n "$(pidof cat)"; then
                if read -r line; then
                    if [ "$line" -eq 1 ]; then
                        echo "OK." > "$COM_PORT"
                        data=$(cat "$ADDR_DATAFILE")
                        echo "Commencing Transmission...." > "$COM_PORT"
                        echo "$data" > "$COM_PORT"
                        echo "Success!"
                    fi
                fi
            else
                echo "Port Not Active."
                exit
            fi
            # --------------------------------------
        done
        exec <&-
    fi
fi
