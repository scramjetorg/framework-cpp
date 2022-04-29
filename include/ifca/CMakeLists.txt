add_subdirectory(transform)

target_sources(IFCA PUBLIC 
	drain.hpp
	exceptions.hpp
	ifca.hpp
	isIfcaInterface.hpp
	maxParallel.hpp
	transformChain.hpp
	types.hpp
)