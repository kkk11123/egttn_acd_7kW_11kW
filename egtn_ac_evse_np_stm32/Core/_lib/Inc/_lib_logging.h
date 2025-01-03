/*
 * _lib_logging.h
 *
 *  Created on: Jan 6, 2023
 *      Author: MSI
 */

#ifndef LIB_INC__LIB_LOGGING_H_
#define LIB_INC__LIB_LOGGING_H_



#include <stdio.h>
//#include <iostream.h>
#include "main.h"
#include "usart.h"

#define _DEBUG__NON_		0
#define _DEBUG__ITM_		1
#define _DEBUG__UART_		2
#define	__DEBUG_PRINTF__	_DEBUG__UART_

#define _LIB_LOGGING_printf		printf

/**
  * @name  		_LOGGING_Print_file_line
  * @parameter  X : first print string
  * @brief 		Represents two types of values in the current thread.
  * 			1). The path and name of the current source file. (__FILE__)
  * 			2). Current thread line number.(__LINE__)
  *
  * 			You can use it usefully before moving on to 'Error_handler()' or when expressing a specific error code.
  * @retval     None
  */
#define _LIB_LOGGING_print_file_line(X)    _LIB_LOGGING_printf("%s : %s , %d line\r\n", (X), __FILE__, __LINE__)

/**
  * @name  		_LOGGING_Print_date_time
  * @parameter  X : first print string
  * @brief 		Represents the date and time of the last compilation.
  *
  *				This can be useful for binary file versioning.
  * @retval     None
  */
#define _LIB_LOGGING_print_date_time(X)    _LIB_LOGGING_printf("%s : %s %s \r\n", (X), __DATE__, __TIME__)


#endif /* LIB_INC__LIB_LOGGING_H_ */
