enum _error_codes
{
    W_OLD_BASELINE = 2,
    W_FILE_NOT_FOUND = 1,
    E_SUCCESS = 0,
    E_SENSOR = -1,
    E_FILE_ACCESS = -2
};

/* type to provide in your API */
typedef enum _error_codes error_t;

/* use this to provide a perror style method to help consumers out */
struct _errordesc {
    error_t  code;
    char *message;
} errordesc[] = {
    { W_OLD_BASELINE, "Baseline is old"},
    { W_FILE_NOT_FOUND, "No such file"},
    { E_SUCCESS, "No error" },
    { E_SENSOR, "Error getting reading from sensor" },
    { E_FILE_ACCESS, "Error accessing file" }
};

void print_error(error_t err)
{
  for(uint16_t i=0; i < sizeof(errordesc)/sizeof(errordesc[0]); i++)
  {
    if (errordesc[i].code == err)
    {
      Serial.println(errordesc[i].message);
    }
  }
}