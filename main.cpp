#include <iostream>

#include "publisher.h"
#include "subscriber.h"

int main()
{
	command_reader reader;
	accumulator acc(3);
	default_printer consolePrinter;
	file_printer fileWriter;

	reader.addSubscriber(&acc);
	acc.addSubscriber(&consolePrinter);
	acc.addSubscriber(&fileWriter);

	reader.run();

	return 0;
}