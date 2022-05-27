add_subdirectory(transform)

target_sources(IFCA PUBLIC 
	exceptions.hpp
	ifca.hpp
	isIfcaInterface.hpp
	maxParallel.hpp
	transformChain.hpp
)