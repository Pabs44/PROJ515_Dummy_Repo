# ----- CHECKING IF ADDRESS FOR DATAFILE EXISTS -----
$ADDR_DATAFILE = "C:\Users\cjenner\Documents\GitHub\PROJ515_Dummy_Repo\ESP32_Arduino_Code\ESP32_Code\data.csv"
# CHECK THROUGH PuTTY.
$COM_PORT = "COM6";

if ($false -match (Test-Path -Path $ADDR_DATAFILE)) {
    Write-Output "Invalid DataFile Link."
    exit
# ---------------------------------------------------
} else {
    # ----- CHECKING IF PORT IS OPERATIONAL ---------
    if ([System.IO.Ports.SerialPort]::getportnames() -notcontains $COM_PORT) {
        Write-Output "Not Available Port."
        exit
    # -----------------------------------------------
    } else {
        $port = new-Object System.IO.Ports.SerialPort $COM_PORT,115200,None,8,one
        $port.Open()
        while ($true) {
            # ----- CHECKING IF PORT IS ACTIVE ------
            # May need improving, to show that communication is actually happening.
            if (Get-WmiObject Win32_SerialPort | Where-Object {$_.DeviceID -eq $COM_PORT}) {
                if (1 -eq $port.ReadLine()) {
                    $port.WriteLine("OK.")
                    $Data = Get-Content $ADDR_DATAFILE
                    $port.WriteLine("Commencing Transmission....")
                    $port.Write($Data)
                    Write-Output "Success!"
                }
            } else {
                Write-Output "Port Not Active."
                exit
            }
            # --------------------------------------
        }
        $port.Close()
    }
}