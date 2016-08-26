/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


/*
 * This method initializes the Config-Interface for the device.
 */
void init_config()
{
#error "Function not implemented."
}


/*
 * This method fills in the JSONified-config-value for "key" into "buffer".
 *
 * It returns the following ::
 *
 * SUCCESS ==> If a config with the specified "key" is found.
 * FAILURE ==> If no config with the specified "key" is found.
 */
int get_config_value_from_persistent_storage(const char *key, char *buffer, int maxBufferLength)
{
#error "Function not implemented."
}


/*
 * This method saves the JSONified-config-value for "key" onto persistent-storage.
 * The example value is of the form ::
 *
 *      {'key' : 'key_value', 'type' : '1', 'val' : 'value', 'desc' : 'description for this config'}
 *
 *
 * Note that for the 'type' field :
 *
 *      '0' denotes that the key-type is of STRING
 *      '1' denotes that the key-type is of INTEGER (although it is stored in stringified-form in 'val' field).
 *
 * It returns the following ::
 *
 * SUCCESS ==> If the config was successfully saved.
 * FAILURE ==> If the config could not be saved.
 */
int save_config_value_on_persistent_storage(const char *key, const char *value)
{
#error "Function not implemented."
}


/*
 * This method deletes the JSONified-config-value for "key" (if at all it exists).
 *
 * It returns the following ::
 *
 * SUCCESS ==> If a config with the specified "key" was found and deleted successfully.
 * FAILURE ==> In every other case.
 */
int delete_config_value_from_persistent_storage(const char *key)
{
#error "Function not implemented."
}


/*
 * This method releases the config, just before the system is going for a reset.
 */
void release_config()
{
#error "Function not implemented."
}
