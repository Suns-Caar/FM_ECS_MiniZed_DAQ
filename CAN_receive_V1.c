/*
/**********
*
* Copyright (C) 2010 - 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
**********/
/**********/
/**
*
* @file xcanps_polled_example.c
*
* Contains an example of how to use the XCanPs driver directly.
* This example shows the using the driver/device in polled mode.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00a xd/sv  01/12/10 First release
* 2.1 adk 		23/08/14 Fixed CR:798792 Peripheral test for CANPS IP in
*						 SDK claims a 40kbps baud rate but it's not.
* </pre>
*
**********/

/**** Include Files ****/

#include "xcanps.h"
#include "xparameters.h"
#include "xil_printf.h"

/*** Constant Definitions ****/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CAN_DEVICE_ID	XPAR_XCANPS_0_DEVICE_ID

/*
 * Maximum CAN frame length in words.
 */
#define XCANPS_MAX_FRAME_SIZE_IN_WORDS (XCANPS_MAX_FRAME_SIZE / sizeof(u32))

#define FRAME_DATA_LENGTH 		8  /* Frame Data field length */

/*
 * Message Id Constant.
 */
#define TEST_MESSAGE_ID			1365

/*
 * The Baud Rate Prescaler Register (BRPR) and Bit Timing Register (BTR)
 * are setup such that CAN baud rate equals 40Kbps, assuming that the
 * the CAN clock is 24MHz. The user needs to modify these values based on
 * the desired baud rate and the CAN clock frequency. For more information
 * see the CAN 2.0A, CAN 2.0B, ISO 11898-1 specifications.
 */

/*
 * Timing parameters to be set in the Bit Timing Register (BTR).
 * These values are for a 40 Kbps baudrate assuming the CAN input clock
 frequency
 * is 24 MHz.
 */
#define TEST_BTR_SYNCJUMPWIDTH		3
#define TEST_BTR_SECOND_TIMESEGMENT	2
#define TEST_BTR_FIRST_TIMESEGMENT	15

/*
 * The Baud rate Prescalar value in the Baud Rate Prescaler Register (BRPR)
 * needs to be set based on the input clock  frequency to the CAN core and
 * the desired CAN baud rate.
 * This value is for a 40 Kbps baudrate assuming the CAN input clock frequency
 * is 24 MHz.
 */
#define TEST_BRPR_BAUD_PRESCALAR	29

/**** Type Definitions *****/

/** Macros (Inline Functions) Definitions ****/

/*** Function Prototypes *****/

int CanPsPolledExample(u16 DeviceId);
//static int SendFrame(XCanPs *InstancePtr);
static int RecvFrame(XCanPs *InstancePtr);

/*** Variable Definitions ****/

/*
 * Buffers to hold frames to send and receive. These are declared as global so
 * that they are not on the stack.
 * These buffers need to be 32-bit aligned
 */
static u32 TxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];
static u32 RxFrame[XCANPS_MAX_FRAME_SIZE_IN_WORDS];


/* Driver instance */
static XCanPs Can;

/**********/
/**
*
* This function is the main function of the Can polled example.
*
* @param	None
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
*********/
#ifndef TESTAPP_GEN
int main()
{
	int Status;

	xil_printf("CAN Polled Mode Example Test \r\n");

	/*
	 * Run the Can Polled example, specify the Device ID that is generated
	 * in xparameters.h .
	 */
	Status = CanPsPolledExample(CAN_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("CAN Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	//xil_printf("Successfully ran CAN Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*********/
/**
*
* The entry point for showing the XCanPs driver in polled mode. The example
* configures the device for internal loop back mode, then sends a Can
* frame, receives the same Can frame, and verifies the frame contents.
*
* @param	DeviceId is the XPAR_<CANPS_instance>_DEVICE_ID value from
*		xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise driver-specific error code.
*
* @note
*
* If the device is not working correctly, this function may enter an infinite
* loop and will never return to the caller.
*
**********/
int CanPsPolledExample(u16 DeviceId)
{
	int Status;
	XCanPs *CanInstPtr = &Can;
	XCanPs_Config *ConfigPtr;

	/*
	 * Initialize the Can device.
	 */
	ConfigPtr = XCanPs_LookupConfig(DeviceId);
	if (CanInstPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XCanPs_CfgInitialize(CanInstPtr,
					ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Enter Configuration Mode so we can setup Baud Rate Prescaler
	 * Register (BRPR) and Bit Timing Register (BTR).
	 */
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_CONFIG);
	while(XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_CONFIG);

	/*
	 * Setup Baud Rate Prescaler Register (BRPR) and
	 * Bit Timing Register (BTR).
	 */
	XCanPs_SetBaudRatePrescaler(CanInstPtr, TEST_BRPR_BAUD_PRESCALAR);
	XCanPs_SetBitTiming(CanInstPtr, TEST_BTR_SYNCJUMPWIDTH,
				TEST_BTR_SECOND_TIMESEGMENT,

				TEST_BTR_FIRST_TIMESEGMENT);
	XCanPs_EnterMode(CanInstPtr, XCANPS_MODE_NORMAL);
	while(XCanPs_GetMode(CanInstPtr) != XCANPS_MODE_NORMAL){
		print("not normal");
		XCanPs_WriteReg(CanInstPtr->CanConfig.BaseAddr,
							XCANPS_MSR_OFFSET, 0U);
					XCanPs_WriteReg(CanInstPtr->CanConfig.BaseAddr,
							XCANPS_SRR_OFFSET, XCANPS_SRR_CEN_MASK);

	}
	int i =0;
	while(XCanPs_GetMode(CanInstPtr) == XCANPS_MODE_NORMAL){
i++;
		print("normal now ");
	if(i==1){

		break;
	}
	print(i);
	}
//	Status = SendFrame(CanInstPtr);
//	if (Status != XST_SUCCESS) {
//		return Status;
//	}
	while(1){
	Status=RecvFrame(CanInstPtr);
	sleep(1);
	}

	return Status;
}


/*********/
/**
*
* Send a CAN frame.
*
* @param	InstancePtr is a pointer to the driver instance
*
* @return	XST_SUCCESS if successful, a driver-specific return code if not.
*
* @note
*
* This function waits until TX FIFO has room for at least one frame before
* sending a frame. So this function may block if the hardware is not built
* correctly.
*
**********/
//static int SendFrame(XCanPs *InstancePtr)
//{
//	u8 *FramePtr;
//	int Index;
//	int Status;
//
//	/*
//	 * Create correct values for Identifier and Data Length Code Register.
//	 */
//	TxFrame[0] = (u32)XCanPs_CreateIdValue((u32)TEST_MESSAGE_ID, 0, 0, 0, 0);
//	TxFrame[1] = (u32)XCanPs_CreateDlcValue((u32)FRAME_DATA_LENGTH);
//
//	/*
//	 * Now fill in the data field with known values so we can verify them
//	 * on receive.
//	 */
//	FramePtr = (u8 *)(&TxFrame[2]);
//	for (Index = 0; Index < FRAME_DATA_LENGTH; Index++) {
//		*FramePtr++ = (u8)Index;
//	}
//
//	/*
//	 * Wait until TX FIFO has room.
//	 */
//	while (XCanPs_IsTxFifoFull(InstancePtr) == TRUE);
//
//	/*
//	 * Now send the frame.
//	 *
//	 * Another way to send a frame is keep calling XCanPs_Send() until it
//	 * returns XST_SUCCESS. No check on if TX FIFO is full is needed anymore
//	 * in that case.
//	 */
//	while(1){
//
//
//	Status = XCanPs_Send(InstancePtr, TxFrame);
////	print("Done\n");
//	sleep(1);
//	}
//
//	return Status;
//}
static int RecvFrame(XCanPs *InstancePtr)
{
	u8 *FramePtr;
	int Status;
	int Index;

	/*
	 * Wait until a frame is received.
	 */
	while (XCanPs_IsRxEmpty(InstancePtr) == TRUE);

	/*
	 * Receive a frame and verify its contents.
	 */
	Status = XCanPs_Recv(InstancePtr, RxFrame);
		print("Message received");
		/*
		 * Verify Identifier and Data Length Code.
		 */
//		if (RxFrame[0] !=
//			(u32)XCanPs_CreateIdValue((u32)TEST_MESSAGE_ID, 0, 0, 0, 0))
//			print("\n ID done ");
//			return XST_LOOPBACK_ERROR;
//
//		if ((RxFrame[1] & ~XCANPS_DLCR_TIMESTAMP_MASK) != TxFrame[1])
//			print("\n DLC done ");
//			return XST_LOOPBACK_ERROR;

		/*
		 * Verify Data field contents.
		 */
		FramePtr = (u8 *)(&RxFrame[0]);
		//xil_printf("\n %s",RxFrame[2]);

		for (Index = 0; Index < 50; Index++) {
				printf("\n %d", FramePtr[8]);
				printf("\n %d", FramePtr[9]);
				printf("\n %d", FramePtr[10]);
				printf("\n %d", FramePtr[11]);
				printf("\n %d", FramePtr[12]);
				printf("\n %d", FramePtr[13]);
				printf("\n %d", FramePtr[14]);
				printf("\n %d", FramePtr[15]);
	}


	return Status;
}

