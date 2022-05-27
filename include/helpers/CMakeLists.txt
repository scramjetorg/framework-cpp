add_subdirectory(Logger)

target_sources(IFCA PUBLIC 
	asyncForwarder.hpp
	crtp.hpp
	functionTraits.hpp
	futureIsReady.hpp
	FWD.hpp
	isStlContainer.hpp
	timer.hpp
)