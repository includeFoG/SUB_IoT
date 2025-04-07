
String localTime = "NoInit_0";
String LogJSON = "{NO INIT}";

bool _failConnect, _configLoaded = false;

int retry = 0;

int contFallos = 0;

int n_Analiz = 2;  // la id del RK900 es el 2



#define WDT_TIMEOUT 480 //SEGUNDOS (8min)

esp_task_wdt_config_t configWDT = {
  .timeout_ms = WDT_TIMEOUT * 1000,
  .idle_core_mask = (1 << 1) - 1,    // Bitmask of all cores
  .trigger_panic = true,
};

float WMaxPrev = 0;
bool  F_BrokerMQTT = false;

long  sum_SecAct = 0; //Sumatorio de lahora actual en segundos

long SendTime = -2147483648 ; //Tiempo trascurrido desde el ultimo envio
unsigned long current_timestamp;    // Actual timestamp read from NTPtime_t now;

bool a_sendTime = true; //flag de alarma de tiempo de envi
