MEMORY
{
  wavefile (r) : ORIGIN = 0x0000, LENGTH = 1000M
}

SECTIONS
{
  /* place wave file data into the wavfile section, located at origin */
  .wavefile : {
    KEEP(*(.wavefile))
  } > wavefile

  /* drop everyting else */
  /DISCARD/ : { *(.*) }
}

