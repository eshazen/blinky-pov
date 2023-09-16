#------------------------------------------------------------
# Oscilloscope display for lab 5.4
#------------------------------------------------------------

from Tkinter import *
import serial
import time

w_width = 500                   # window width
w_height = 400                  # window height

record_length = 100             # number of points in a waveform

# scale for ADC values 0-1023 to fit in height
vscale = float(w_height)/1024
# scale for record_length points to fit in w_width
hscale = float(w_width)/record_length

root = Tk()                     # create top-level Tk object

#---------------------------------------------------------------
# do_trigger():  trigger the scope and display a waveform
#---------------------------------------------------------------
def do_trigger():

    ser.write( "t")         # send trigger character to Arduino

    data = []               # create an empty list to store data

    # loop to read the data points
    for i in range( record_length):    
        s = ser.readline()  # read one line from the Arduino
        v = int(s)          # convert to integer
        data.append( v)     # add to the list

    w.delete(ALL)           # delete everything in the window

    for i in range(1,record_length):   # loop to plot the data points
        w.create_line( hscale*(i-1), w_height-vscale*data[i-1],
                       hscale*i, w_height-vscale*data[i])

    w.after( 100, do_trigger)   # trigger again after 100ms

#--- end of do_trigger() function -----------------------------


#-------------------- main program --------------------

# connect to the Arduino via the serial port
# change the serial port name as required (to i.e. 'COM6' if on windows)
ser = serial.Serial( '/dev/ttyUSB0', 9600, timeout=1)

# the Arduino reboots when you do this, so a delay is needed
# time.sleep( 3.0)

# create a "Canvas" object we can draw on as a child of the root
w = Canvas( root, width=w_width, height=w_height)
w.pack()                        # display it

# trigger once to get things going
do_trigger()

mainloop()                 # start the main event loop
