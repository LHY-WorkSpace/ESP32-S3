file(GLOB_RECURSE SOURCES 	./*.c 
							)

idf_component_register(SRCS	${SOURCES}
                   INCLUDE_DIRS "./"
                    REQUIRES  driver
                              )

# idf_component_register(SRC_DIRS	"Drivers"
#                    INCLUDE_DIRS "./"
#                     REQUIRES  
#                             driver
#                         )


                        