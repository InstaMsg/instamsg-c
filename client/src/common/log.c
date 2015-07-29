/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "./include/log.h"


Logger* get_new_logger(void *arg)
{
    Logger *logger = (Logger*)malloc(sizeof(Logger));

    // Here, physical medium is a file-system.
	logger->medium = get_new_file_system(arg);

    return logger;
}


void release_logger(Logger *logger)
{
    release_file_system(logger->medium);

    // Free the dynamically-allocated memory
    free(logger);

    // TODO: This printf statement should not be here.
    printf("Complete LOG structure, including the underlying physical-medium.. cleaned.\n");
}
