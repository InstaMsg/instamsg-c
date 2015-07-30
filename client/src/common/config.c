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
#include <string.h>

#include "./include/config.h"

Config* get_new_config(void *arg)
{
    Config *config = (Config*)malloc(sizeof(Config));

    // Here, physical medium is a file-system.
	config->medium = get_new_file_system(arg);

    return config;
}


void release_config(Config *config)
{
    release_file_system(config->medium);

    // Free the dynamically-allocated memory
    if(config != NULL)
    {
        free(config);
    }
}


void readConfig(Config *config, const unsigned char *key, ValueType valueType, void *value)
{
}
