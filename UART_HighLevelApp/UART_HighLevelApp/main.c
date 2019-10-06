/* Copyright (c) Microsoft Corporation. All rights reserved.
   Licensed under the MIT License. */

// This sample C application for Azure Sphere demonstrates how to use a UART (serial port).
// The sample opens a UART with a baud rate of 115200. Pressing a button causes characters
// to be sent from the device over the UART; data received by the device from the UART is echoed to
// the Visual Studio Output Window.
//
// It uses the API for the following Azure Sphere application libraries:
// - UART (serial port)
// - GPIO (digital input for button)
// - log (messages shown in Visual Studio's Device Output window during debugging)

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>


// applibs_versions.h defines the API struct versions to use for applibs APIs.
#include "applibs_versions.h"
#include "epoll_timerfd_utilities.h"
#include <applibs/uart.h>
#include <applibs/gpio.h>
#include <applibs/log.h>

// Azure IoT SDK
#include <iothub_client_core_common.h>
#include <iothub_device_client_ll.h>
#include <iothub_client_options.h>
#include <iothubtransportmqtt.h>
#include <iothub.h>
#include <azure_sphere_provisioning.h>
#include "parson.h" // used to parse Device Twin messages.
#include "secrets.h" // Import Azure Sphere IoT Hub Creds

// By default, this sample is targeted at the MT3620 Reference Development Board (RDB).
// This can be changed using the project property "Target Hardware Definition Directory".
// This #include imports the sample_hardware abstraction from that hardware definition.
#include <hw\mt3620_rdb.h>
#include <hw\sample_hardware.h>

// File descriptors - initialized to invalid value
static int uartFd = -1;
static int gpioButtonFd = -1;
static int gpioButtonTimerFd = -1;
static int epollFd = -1;
static int azureTimerFd = -1;
static const int keepalivePeriodSeconds = 20;
// State variables
static GPIO_Value_Type buttonState = GPIO_Value_High;

// Termination state
static volatile sig_atomic_t terminationRequired = false;

// Define custom functions without using a header file
static void DoSomething(int action);
int action = 0;

// Define stuff for connecting to web
static bool iothubConnected = false;
static const size_t retryTimeoutSeconds = 0;
static int azureIoTPollPeriodSeconds = -1;
static IOTHUB_DEVICE_CLIENT_LL_HANDLE iothubClientHandle = NULL;
static const int AzureIoTDefaultPollPeriodSeconds = 5;

static void SetupAzureClient(void);
static void TwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload, size_t payloadSize, void* userContextCallback);
static void HubConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback);

// const char connectionString = "";

/// <summary>
///     Signal handler for termination requests. This handler must be async-signal-safe.
/// </summary>
static void TerminationHandler(int signalNumber)
{
    // Don't use Log_Debug here, as it is not guaranteed to be async-signal-safe.
    terminationRequired = true;
}

/// <summary>
///     Helper function to send a fixed message via the given UART.
/// </summary>
/// <param name="uartFd">The open file descriptor of the UART to write to</param>
/// <param name="dataToSend">The data to send over the UART</param>
static void SendUartMessage(int uartFd, const char *dataToSend)
{
    size_t totalBytesSent = 0;
    size_t totalBytesToSend = strlen(dataToSend);
    int sendIterations = 0;
    while (totalBytesSent < totalBytesToSend) {
        sendIterations++;

        // Send as much of the remaining data as possible
        size_t bytesLeftToSend = totalBytesToSend - totalBytesSent;
        const char *remainingMessageToSend = dataToSend + totalBytesSent;
        ssize_t bytesSent = write(uartFd, remainingMessageToSend, bytesLeftToSend);
        if (bytesSent < 0) {
            Log_Debug("ERROR: Could not write to UART: %s (%d).\n", strerror(errno), errno);
            terminationRequired = true;
            return;
        }

        totalBytesSent += (size_t)bytesSent;
    }

    Log_Debug("Sent %zu bytes over UART in %d calls.\n", totalBytesSent, sendIterations);
}

/// <summary>
///     Handle button timer event: if the button is pressed, send data over the UART.
/// </summary>
static void ButtonTimerEventHandler(EventData *eventData)
{
    if (ConsumeTimerFdEvent(gpioButtonTimerFd) != 0) {
        terminationRequired = true;
        return;
    }

    // Check for a button press
    GPIO_Value_Type newButtonState;
    int result = GPIO_GetValue(gpioButtonFd, &newButtonState);
    if (result != 0) {
        Log_Debug("ERROR: Could not read button GPIO: %s (%d).\n", strerror(errno), errno);
        terminationRequired = true;
        return;
    }

    // If the button has just been pressed, send data over the UART
    // The button has GPIO_Value_Low when pressed and GPIO_Value_High when released
    if (newButtonState != buttonState) {
        if (newButtonState == GPIO_Value_Low) {
            SendUartMessage(uartFd, "Hello world!\n");
        }
        buttonState = newButtonState;
    }
}

/// <summary>
///     Handle UART event: if there is incoming data, print it.
/// </summary>
static void UartEventHandler(EventData *eventData)
{
    const size_t receiveBufferSize = 256;
    uint8_t receiveBuffer[receiveBufferSize + 1]; // allow extra byte for string termination
    ssize_t bytesRead;

    // Read incoming UART data. It is expected behavior that messages may be received in multiple
    // partial chunks.
    bytesRead = read(uartFd, receiveBuffer, receiveBufferSize);
    if (bytesRead < 0) {
        Log_Debug("ERROR: Could not read UART: %s (%d).\n", strerror(errno), errno);
        terminationRequired = true;
        return;
    }

    if (bytesRead > 0) {
        // Null terminate the buffer to make it a valid string, and print it
        receiveBuffer[bytesRead] = 0;
		char * data_recieved = (char *)receiveBuffer;
        Log_Debug("UART received %d bytes: '%s'", bytesRead, data_recieved);

		DoSomething(action);

		Log_Debug("\n");
    }
}

static void DoSomething(int action) {
	Log_Debug("Beginning Action...");
	switch (action) {
		case 1 :
			Log_Debug("Received command to perform action 1 (ONE)!!!");
		case 2 :
			Log_Debug("Received command to perform action 2 (TWO)!!!");
	default:
		Log_Debug("\nNo action received!!!");
	}
}

// event handler data structures. Only the event handler field needs to be populated.
static EventData buttonEventData = {.eventHandler = &ButtonTimerEventHandler};
static EventData uartEventData = {.eventHandler = &UartEventHandler};

/// <summary>
///     Set up SIGTERM termination handler, initialize peripherals, and set up event handlers.
/// </summary>
/// <returns>0 on success, or -1 on failure</returns>
static int InitPeripheralsAndHandlers(void)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = TerminationHandler;
    sigaction(SIGTERM, &action, NULL);

    epollFd = CreateEpollFd();
    if (epollFd < 0) {
        return -1;
    }

    // Create a UART_Config object, open the UART and set up UART event handler
    UART_Config uartConfig;
    UART_InitConfig(&uartConfig);
    uartConfig.baudRate = 115200;
    uartConfig.flowControl = UART_FlowControl_None;
    uartFd = UART_Open(SAMPLE_UART, &uartConfig);
    if (uartFd < 0) {
        Log_Debug("ERROR: Could not open UART: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    if (RegisterEventHandlerToEpoll(epollFd, uartFd, &uartEventData, EPOLLIN) != 0) {
        return -1;
    }

    // Open button GPIO as input, and set up a timer to poll it
    Log_Debug("Opening SAMPLE_BUTTON_1 as input.\n");
    gpioButtonFd = GPIO_OpenAsInput(SAMPLE_BUTTON_1);
    if (gpioButtonFd < 0) {
        Log_Debug("ERROR: Could not open button GPIO: %s (%d).\n", strerror(errno), errno);
        return -1;
    }
    struct timespec buttonPressCheckPeriod = {0, 1000000};
    gpioButtonTimerFd =
        CreateTimerFdAndAddToEpoll(epollFd, &buttonPressCheckPeriod, &buttonEventData, EPOLLIN);
    if (gpioButtonTimerFd < 0) {
        return -1;
    }
	
	// Set up connection to Azure Sphere IoT hub
	SetupAzureClient();

    return 0;
}

/// <summary>
///     Close peripherals and handlers.
/// </summary>
static void ClosePeripheralsAndHandlers(void)
{
    Log_Debug("Closing file descriptors.\n");
    CloseFdAndPrintError(gpioButtonTimerFd, "ButtonTimer");
    CloseFdAndPrintError(gpioButtonFd, "GpioButton");
    CloseFdAndPrintError(uartFd, "Uart");
    CloseFdAndPrintError(epollFd, "Epoll");
}

/// <summary>
///     Main entry point for this application.
/// </summary>
int main(int argc, char *argv[])
{
    Log_Debug("UART application starting.\n");
    if (InitPeripheralsAndHandlers() != 0) {
        terminationRequired = true;
    }

    // Use epoll to wait for events and trigger handlers, until an error or SIGTERM happens
    while (!terminationRequired) {
        if (WaitForEventAndCallHandler(epollFd) != 0) {
            terminationRequired = true;
        }
    }

    ClosePeripheralsAndHandlers();
    Log_Debug("Application exiting.\n");
    return 0;
}

static void SetupAzureClient(void)
{
	if (iothubClientHandle != NULL) {
		IoTHubDeviceClient_LL_Destroy(iothubClientHandle);
	}

	iothubClientHandle =
		IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);

	if (iothubClientHandle == NULL) {
		Log_Debug("couldn't create the client from connection string");
		return;
	}

	IOTHUB_CLIENT_RESULT azureRes =
		IoTHubDeviceClient_LL_SetOption(iothubClientHandle, "TrustedCerts", AzureIoTCertificatesX);
	if (azureRes != IOTHUB_CLIENT_OK) {
		Log_Debug("ERROR: failure to set option \"TrustedCerts\"\n");
		return;
	}

	// Successfully connected, so make sure the polling frequency is back to the default
	azureIoTPollPeriodSeconds = AzureIoTDefaultPollPeriodSeconds;
	struct timespec azureTelemetryPeriod = { azureIoTPollPeriodSeconds, 0 };
	SetTimerFdToPeriod(azureTimerFd, &azureTelemetryPeriod);

	iothubConnected = true;

	if (IoTHubDeviceClient_LL_SetOption(iothubClientHandle, OPTION_KEEP_ALIVE,
		&keepalivePeriodSeconds) != IOTHUB_CLIENT_OK) {
		Log_Debug("ERROR: failure setting option \"%s\"\n", OPTION_KEEP_ALIVE);
		return;
	}

	if (IoTHubDeviceClient_LL_SetDeviceTwinCallback(iothubClientHandle, TwinCallback, NULL) !=
		IOTHUB_CLIENT_OK) {
		Log_Debug("ERROR: failure setting option TwinCallback\n");
		return;
	}

	if (IoTHubDeviceClient_LL_SetConnectionStatusCallback(
		iothubClientHandle, HubConnectionStatusCallback, NULL) != IOTHUB_CLIENT_OK) {
		Log_Debug("ERROR: failure setting option HubConnectionStatusCallback\n");
		return;
	}

	// Set retry policy for the connection to the IoT Hub.
	if (IoTHubDeviceClient_LL_SetRetryPolicy(iothubClientHandle, IOTHUB_CLIENT_RETRY_INTERVAL,
		retryTimeoutSeconds) != IOTHUB_CLIENT_OK) {
		Log_Debug("ERROR: failure setting retry policy\n");
		return;
	}

	IoTHubDeviceClient_LL_DoWork(iothubClientHandle);
}
static const char* GetReasonString(IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason)
{
	static char* reasonString = "unknown reason";
	switch (reason) {
	case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
		reasonString = "IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN";
		break;
	case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
		reasonString = "IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED";
		break;
	case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
		reasonString = "IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL";
		break;
	case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
		reasonString = "IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED";
		break;
	case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
		reasonString = "IOTHUB_CLIENT_CONNECTION_NO_NETWORK";
		break;
	case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
		reasonString = "IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR";
		break;
	case IOTHUB_CLIENT_CONNECTION_OK:
		reasonString = "IOTHUB_CLIENT_CONNECTION_OK";
		break;
	}
	return reasonString;
}
static void HubConnectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result,
	IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
	void* userContextCallback)
{
	iothubConnected = (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED);
	if (iothubConnected)
		Log_Debug("INFO - HubConnectionStatusCallback: connection to the IoT Hub has been established.\n");
	if (!iothubConnected) {
		//Log_Debug("INFO: IoT Hub connection is down, retrying connection in 5 seconds...\n");
		Log_Debug("IoT Hub Connection Failed: %s\n", GetReasonString(reason));
	}
}

static void TwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char* payload,
	size_t payloadSize, void* userContextCallback)
{
	size_t nullTerminatedJsonSize = payloadSize + 1;
	char* nullTerminatedJsonString = (char*)malloc(nullTerminatedJsonSize);
	if (nullTerminatedJsonString == NULL) {
		Log_Debug("ERROR: Could not allocate buffer for twin update payload.\n");
		abort();
	}

	// Copy the provided buffer to a null terminated buffer.
	memcpy(nullTerminatedJsonString, payload, payloadSize);
	// Add the null terminator at the end.
	nullTerminatedJsonString[nullTerminatedJsonSize - 1] = 0;

	JSON_Value* rootProperties = NULL;
	rootProperties = json_parse_string(nullTerminatedJsonString);
	if (rootProperties == NULL) {
		Log_Debug("WARNING: Cannot parse the string as JSON content.\n");
		goto cleanup;
	}

	JSON_Object* rootObject = json_value_get_object(rootProperties);
	JSON_Object* desiredProperties = json_object_dotget_object(rootObject, "desired");
	if (desiredProperties == NULL) {
		desiredProperties = rootObject;
	}

	// Handle the Device Twin Desired Properties here.
	/*JSON_Object* LEDState = json_object_dotget_object(desiredProperties, "StatusLED");
	if (LEDState != NULL) {
		statusLedOn = (bool)json_object_get_boolean(LEDState, "value");
		GPIO_SetValue(deviceTwinStatusLedGpioFd,
			(statusLedOn == true ? GPIO_Value_Low : GPIO_Value_High));
		TwinReportBoolState("StatusLED", statusLedOn);
	}

	JSON_Object* OvenStartLEDState = json_object_dotget_object(desiredProperties, "RipleyOvenOnLED");
	if (OvenStartLEDState != NULL) {
		ovenLedOn = (bool)json_object_get_boolean(OvenStartLEDState, "value");
		GPIO_SetValue(deviceTwinRipleyLedGpioFd,
			(ovenLedOn == true ? GPIO_Value_Low : GPIO_Value_High));
		TwinReportBoolState("RipleyOvenOnLED", ovenLedOn);
	}*/

cleanup:
	// Release the allocated memory.
	json_value_free(rootProperties);
	free(nullTerminatedJsonString);
}