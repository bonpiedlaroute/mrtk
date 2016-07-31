
#ifndef MRTK_PRIOTABLE
#define MRTK_PRIOTABLE

/* number of group of tasks in queue */
#define NB_GROUP  8


extern Uint32 RdyPrioTable[NB_GROUP];

extern Uint8 RdyGrp;


extern Uint32 RunPrioTable[NB_GROUP];

extern Uint8 RunGrp;

extern Uint32 mrtk_findhghbitsetonright32(Uint32 ip_value);

extern Uint8 mrtk_findhghbitsetonright8(Uint8 ip_value);

extern Uint32 mrtk_findhghbitsetonleft32(Uint32 ip_value);

extern Uint32 mrtk_findhghbitsetonleft8(Uint8 ip_value);

extern void mrtk_InitPrioTable();

#endif
