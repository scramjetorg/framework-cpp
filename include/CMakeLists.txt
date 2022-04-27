
target_sources(IFCA PUBLIC 
	helpers/Logger/emptyLogger.hpp
	helpers/Logger/logger.hpp
	helpers/Logger/loggerImpl.hpp
	helpers/Logger/logMessage.hpp
	helpers/Logger/terminalColorCodes.hpp
	helpers/Logger/time.hpp

	helpers/asyncForwarder.hpp
	helpers/crtp.hpp
	helpers/functionTraits.hpp
	helpers/futureIsReady.hpp
	helpers/FWD.hpp
	helpers/threadList.hpp
	helpers/timer.hpp

	ifca/transform/each.hpp
	ifca/transform/filter.hpp
	ifca/transform/from.hpp
	ifca/transform/into.hpp
	ifca/transform/isTransformExpression.hpp
	ifca/transform/map.hpp
	ifca/transform/orderedExpression.hpp
	ifca/transform/outStream.hpp
	ifca/transform/race.hpp
	ifca/transform/reduce.hpp
	ifca/transform/run.hpp
	ifca/transform/transformExpression.hpp

	ifca/drain.hpp
	ifca/exceptions.hpp
	ifca/ifca.hpp
	ifca/isIfcaInterface.hpp
	ifca/maxParallel.hpp
	ifca/transformChain.hpp
	ifca/types.hpp

	streams/dataStream.hpp

)