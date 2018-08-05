 # Keypad_Controller.py
 # Date: June-10-2018
 # Created by: Ryan Lehman

 from MCP23008 import MCP23008
 
class keypad(MCP23008):

    # Setup for variables
    INPUT       = 0
    OUTPUT      = 1
    HIGH        = 1
    LOW         = 0
     
    KEYPAD = [
    [1,2,3],
    [4,5,6],
    [7,8,9],
    ["*",0,"#"]
    ]
     
    ROW         = [6,5,4,3]
    COLUMN      = [2,1,0]
     
    def __init__(self, address=0x20, num_gpios=8):
        self.mcp2 = MCP23008(address, num_gpios)
         
    def getValue(self):
         
        # Set all columns as output low
        for j in range(len(self.COLUMN)):
            self.mcp2.config(self.COLUMN[j], self.mcp2.OUTPUT)
            self.mcp2.output(self.COLUMN[j], self.LOW)
         
        # Set all rows as input
        for i in range(len(self.ROW)):
            self.mcp2.config(self.ROW[i], self.mcp2.INPUT)
            self.mcp2.addPullup(self.ROW[i], True)
         
        # Scan rows for pushed key/button
        rowVal = -1
        for i in range(len(self.ROW)):
            tmpRead = self.mcp2.input(self.ROW[i])

            if tmpRead == 0:

                rowVal = i

        # if rowVal is (-1) then exit
        if rowVal == -1:
            self.exit()
            return

        # Set all rows as output low
        for j in range(len(self.ROW)):
            self.mcp2.config(self.ROW[j], self.mcp2.OUTPUT)
            self.mcp2.output(self.ROW[j], self.LOW)
         
        # Set all columns as input
        for i in range(len(self.COLUMN)):
            self.mcp2.config(self.COLUMN[i], self.mcp2.INPUT)
            self.mcp2.addPullup(self.COLUMN[i], True)
         
        # Scan columns for key/button
        colVal = -1
        for j in range(len(self.COLUMN)):
            tmpRead = self.mcp2.input(self.COLUMN[j])
            
            if tmpRead == 0:
                print("tempRead" + str(tmpRead))
                colVal=j
                break
         
        if colVal == -1:
            self.exit()
            return
               
        # Return the value of the key pressed
        self.exit()   
        return self.KEYPAD[rowVal][colVal]
             
    def exit(self):
        # Reinitialize all rows and columns as input before exiting
        for i in range(len(self.ROW)):
                self.mcp2.config(self.ROW[i], self.INPUT) 
        for j in range(len(self.COLUMN)):
                self.mcp2.config(self.COLUMN[j], self.INPUT)
