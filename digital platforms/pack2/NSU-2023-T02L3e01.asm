asect  0x00

# =================================
# INSERT YOUR CODE BELOW
# Executable instructions only
# No dc or ds pseudo-instructions
# Do not include a halt instruction: that has been done already (below)
# ---------------------------------------------------------------------




# =================================
# LEAVE THIS PART OF THE FILE ALONE
# Do not change the next two instructions: they must be the last two
# instructions executed by your program.
    ldi r0, ans  # Loads the address of your result into r0 for the robot
    halt         # Brings execution to a halt

# =================================
# DATA GOES BELOW
# We have set this up for you, but you will need to test your program by
# compiling and running it several times with different input data values
# (different bit-strings placed at address x)
# ---------------------------------------------------------------------

INPUTS>
a:    dc  0x23,0xa8   # replace two bytes by your choice of bit-srings for testing
b:    dc  0xf3,0x07   # replace two bytes by your choice of bit-srings for testing

ENDINPUTS>

ans:  ds 2      # two byte reserved for the result
end


