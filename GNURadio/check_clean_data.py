#from rflib import *
import sys, bitstring
import numpy as np
 
"""
1010 1010 1010 1010 1010 1010 1010 1010 PREAMBLE
0101 0111 0100 0011 SYNC WORD (x2)
0000 0111 0000 0111 0000 0111 0000 0111 0000 0111 0000 0111 DATA (0x07 x6)
""" 


hexa_07_stream = "1010101010101010101010101010101001010111010000110101011101000011000001110000011100000111000001110000011100000111"

class Check:
    def __init__(self, stream):
        self.pos_counter = 0
        self.tmp_pos_counter = 0
        self.pc= 0
        
        self.counter = 0
        self.stream = stream
        self.slen = len(stream)
        self.max_counter = 0
        return
    
    def bit_string(self, b):
        if(b==self.stream[self.counter]):
            self.counter+=1
            if(self.counter>self.max_counter):
                self.max_counter = self.counter
                self.pc=self.pos_counter
        else:
            self.counter = 0
            self.pos_counter = self.tmp_pos_counter
        self.tmp_pos_counter+=1
        return self.counter==self.slen

def frame_generator():
    preamble = "10101010101010101010101010101010"
    double_sync_word = "01010111010000110101011101000011"
    
    size = 54
    data = preamble+double_sync_word
    for i in range(size):
        data+='{0:08b}'.format(i%256)
        #TODO generate binary data
    return data
    
def get_bitstring(symbols):
    bstring = frame_generator();
    print(bstring)
    check = Check(bstring)
    bs = ''
    b = ''
    for s in symbols:
        if s > 0.6:
            b = '1'
            bs += b
        else:
            b = '0'
            bs += b

        if(check.bit_string(b)):
            print("success")
            break
    print(check.max_counter," bits out of ",check.slen)
    print(int(check.max_counter/8)," bytes out of ",int(check.slen/8))
    print("Found in position: ",check.pos_counter)
    if(check.max_counter!=check.slen): print(bs[check.pos_counter:-1])
        

if __name__=="__main__":
    with open(sys.argv[1], 'rb') as infile:
        data = np.fromfile(infile, np.float32)
        data = list(data)
        get_bitstring(data)
        
	
