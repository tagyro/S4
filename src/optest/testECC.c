//
//  testECC.c
//  S4
//
//  Created by vincent Moscaritolo on 11/2/15.
//  Copyright © 2015 4th-A Technologies, LLC. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "s4.h"
#include "optest.h"



static S4Err sTestECC(int keySize)
{
#define PTsize 32
    
    S4Err     err = kS4Err_NoErr;
    int     i;
    
    uint8_t        PT[PTsize];
    
    uint8_t        CT[256];
    size_t         CTlen = 0;
    
    uint8_t        DT[PTsize];
    size_t         DTlen = 0;
    
    uint8_t         pubKey[256];
    size_t          pubKeyLen = 0;
    
    uint8_t         privKey[256];
    size_t          privKeyLen = 0;
 
    uint8_t         keyHash[32];
    size_t          keyHashLen = 0;
    
    bool isPrivate = false;
    size_t  importKeySize = 0;
    bool    isANSIx963 = false;
    
    //   uint8_t             tempBuf[256];
    //   unsigned long       tempLen;
    
    
    OPTESTLogInfo("\tECC-%d \n",  keySize);
    
    ECC_ContextRef ecc = kInvalidECC_ContextRef;
    ECC_ContextRef eccPub = kInvalidECC_ContextRef;
    
    // fill PT
    for(i = 0; i< PTsize; i++) PT[i]= i;
    
    err = ECC_Init(&ecc);
    
    OPTESTLogVerbose("\t\tGenerate Pub Key (%ld bytes)\n", pubKeyLen);
    err = ECC_Generate(ecc, keySize); CKERR;
    
    err =  ECC_Export_ANSI_X963( ecc, pubKey, sizeof(pubKey), &pubKeyLen);CKERR;
    OPTESTLogVerbose("\t\tExport Public Key (%ld bytes)\n", pubKeyLen);
    dumpHex(IF_LOG_DEBUG, pubKey,  (int)pubKeyLen, 0);
    
    err = ECC_Import_Info( pubKey, pubKeyLen, &isPrivate, &isANSIx963, &importKeySize );CKERR;
    OPTESTLogVerbose("\t\t\t%d bit %s%s key\n", (int)importKeySize , isANSIx963 ?"ANSI x9.63 ":"", isPrivate ?"private":"public");
    
    err =  ECC_PubKeyHash(ecc, keyHash, sizeof(keyHash), &keyHashLen);CKERR;
    OPTESTLogVerbose("\t\tKey Hash (%ld bytes)\n", keyHashLen);
    dumpHex(IF_LOG_DEBUG, keyHash,  (int)keyHashLen, 0);

    err =  ECC_Export(ecc, true, privKey, sizeof(privKey), &privKeyLen);CKERR;
    OPTESTLogVerbose("\t\tExport Private Key (%ld bytes)\n", privKeyLen);
    dumpHex(IF_LOG_DEBUG, privKey,  (int)privKeyLen, 0);
    
    err = ECC_Import_Info( privKey, privKeyLen, &isPrivate, &isANSIx963, &importKeySize );CKERR;
    OPTESTLogVerbose("\t\t\t%d bit %s%s key\n", (int)importKeySize , isANSIx963 ?"ANSI x9.63 ":"", isPrivate ?"private":"public");
    
    // delete keys
    if(ECC_ContextRefIsValid(ecc) ) ECC_Free(ecc );
    ecc = kInvalidECC_ContextRef;
    
    err = ECC_Init(&eccPub);
    err = ECC_Import_ANSI_X963( eccPub, pubKey, pubKeyLen);CKERR;
    
    importKeySize = 0;
    err =  ECC_KeySize(eccPub, &importKeySize);
    OPTESTLogVerbose("\t\tImported %d bit public key\n", (int)importKeySize  );
    
    err = ECC_Encrypt(eccPub, PT, sizeof(PT),  CT, sizeof(CT), &CTlen);CKERR;
    OPTESTLogVerbose("\t\tEncrypt message: (%ld bytes)\n", CTlen);
    dumpHex(IF_LOG_DEBUG, CT,  (int)CTlen, 0);
    
    err = ECC_Init(&ecc);
    err = ECC_Import(ecc, privKey, privKeyLen);CKERR;
    
    err =  ECC_KeySize(ecc, &importKeySize);
    OPTESTLogVerbose("\t\tImported %d bit private key\n", (int)importKeySize  );
    
    err =  ECC_PubKeyHash(ecc, keyHash, sizeof(keyHash), &keyHashLen);CKERR;
    OPTESTLogVerbose("\t\tKey Hash (%ld bytes)\n", keyHashLen);
    dumpHex(IF_LOG_DEBUG, keyHash,  (int)keyHashLen, 0);

    err = ECC_Decrypt(ecc, CT, CTlen,  DT, sizeof(DT), &DTlen); CKERR;
    
    /* check against know-answer */
    err= compareResults( DT, PT, PTsize , kResultFormat_Byte, "ECC Decrypt"); CKERR;
    OPTESTLogVerbose("\t\tDecrypted OK\n");
    dumpHex(IF_LOG_DEBUG, DT,  (int)DTlen, 0);
    
    err = ECC_Sign(ecc, PT, sizeof(PT),  CT, sizeof(CT), &CTlen);CKERR;
    OPTESTLogVerbose("\t\tSigned message (%ld bytes)\n", CTlen);
    dumpHex(IF_LOG_DEBUG, CT,  (int)CTlen, 0);
    
    err = ECC_Verify(ecc, CT, CTlen, PT, sizeof(PT));
    OPTESTLogVerbose("\t\tVerify = %s\n",  IsS4Err(err)?"fail":"pass");
    
    PT[3]= 9;
    err = ECC_Verify(ecc, CT, CTlen, PT, sizeof(PT));
    OPTESTLogVerbose("\t\tVerify bad packet = %s\n",  IsS4Err(err)?"fail":"pass");
    if(err == kS4Err_BadIntegrity) err = kS4Err_NoErr;
    
    OPTESTLogVerbose("\n");
done:
    
    if(ECC_ContextRefIsValid(ecc) ) ECC_Free(ecc );
    if(ECC_ContextRefIsValid(eccPub)) ECC_Free(eccPub);
    
    return err;
    
}

S4Err sTestECC_DH(int keySize)
{
    
    S4Err     err = kS4Err_NoErr;
    
    uint8_t         pubKey1[256];
    size_t          pubKeyLen1 = 0;
    uint8_t         privKey1[256];
    size_t          privKeyLen1 = 0;
    
    uint8_t         pubKey2[256];
    size_t          pubKeyLen2 = 0;
    uint8_t         privKey2[256];
    size_t          privKeyLen2 = 0;
 
    uint8_t         curveName[256];
    size_t          curveNameLen = 0;
    
    ECC_ContextRef eccPriv = kInvalidECC_ContextRef;
    ECC_ContextRef eccPub  = kInvalidECC_ContextRef;
    
    uint8_t         Z1       [256];
    size_t          Zlen1;
    uint8_t         Z2       [256];
    size_t          Zlen2;
    
    
    OPTESTLogInfo("\tTesting ECC-DH (%d)\n",  keySize);
    
    /* create keys   */
    OPTESTLogDebug("\t\tGenerate Key 1\n");
    err = ECC_Init(&eccPriv); CKERR;
    err = ECC_Generate(eccPriv, keySize ); CKERR;
    
    err =  ECC_Export_ANSI_X963( eccPriv, pubKey1, sizeof(pubKey1), &pubKeyLen1);CKERR;
    err =  ECC_Export(eccPriv, true, privKey1, sizeof(privKey1), &privKeyLen1);CKERR;
    
    err = ECC_CurveName(eccPriv, curveName, sizeof(curveName), & curveNameLen); CKERR;
   
    OPTESTLogDebug("\t\tKey 1 Pub/Priv %s (%ld,%ld) bytes\n", curveName, pubKeyLen1, privKeyLen1);
    OPTESTLogDebug("\t\tPublic\n");
    dumpHex(IF_LOG_DEBUG, pubKey1,  (int)pubKeyLen1, 0);
    OPTESTLogDebug("\t\tPrivate\n");
    dumpHex(IF_LOG_DEBUG, privKey1,  (int)privKeyLen1, 0);
    OPTESTLogDebug("\n");
    
    if(ECC_ContextRefIsValid(eccPriv) ) ECC_Free(eccPriv );
    eccPriv = kInvalidECC_ContextRef;
    
    
    OPTESTLogDebug("\t\tGenerate Key 2\n");
    err = ECC_Init(&eccPriv); CKERR;
    err = ECC_Generate(eccPriv, keySize ); CKERR;
    
    err =  ECC_Export_ANSI_X963( eccPriv, pubKey2, sizeof(pubKey2), &pubKeyLen2);CKERR;
    err =  ECC_Export(eccPriv, true, privKey2, sizeof(privKey2), &privKeyLen2);CKERR;
    
    OPTESTLogDebug("\t\tKey 2 Pub/Priv  (%ld,%ld) bytes\n", pubKeyLen2, privKeyLen2);
    OPTESTLogDebug("\t\tPublic\n");
    dumpHex(IF_LOG_DEBUG, pubKey2,  (int)pubKeyLen2, 0);
    OPTESTLogDebug("\t\tPrivate\n");
    dumpHex(IF_LOG_DEBUG, privKey2,  (int)privKeyLen2, 0);
    OPTESTLogDebug("\n");
    
    // delete keys
    if(ECC_ContextRefIsValid(eccPriv) ) ECC_Free(eccPriv );
    eccPriv = kInvalidECC_ContextRef;
    
    OPTESTLogDebug("\t\tCalculate Secret for Key1 -> Key2\n");
    err = ECC_Init(&eccPriv);
    err = ECC_Import(eccPriv, privKey1, privKeyLen1);CKERR;
    
    err = ECC_Init(&eccPub);
    err = ECC_Import_ANSI_X963( eccPub, pubKey2, pubKeyLen2);CKERR;
    
    /* Kdk = MAC(Htotal,Z)    where Z is the DH of Pki and PKr */
    Zlen1 = sizeof(Z1);
    err = ECC_SharedSecret(eccPriv, eccPub, Z1, sizeof(Z1), &Zlen1);CKERR;
    
    OPTESTLogVerbose("\t\tECC Shared Secret (Z1):  (%ld bytes)\n",Zlen1);
    dumpHex(IF_LOG_DEBUG, Z1,  (int)Zlen1 , 0);
    OPTESTLogDebug("\n");
    
    
    // delete keys
    if(ECC_ContextRefIsValid(eccPriv) ) ECC_Free(eccPriv );
    eccPriv = kInvalidECC_ContextRef;
    // delete keys
    if(ECC_ContextRefIsValid(eccPub) ) ECC_Free(eccPub );
    eccPub = kInvalidECC_ContextRef;
    
    OPTESTLogDebug("\t\tCalculate Secret for Key2 -> Key1\n");
    err = ECC_Init(&eccPriv);
    err = ECC_Import(eccPriv, privKey2, privKeyLen2);CKERR;
    
    err = ECC_Init(&eccPub);
    err = ECC_Import_ANSI_X963( eccPub, pubKey1, pubKeyLen1);CKERR;
    
    /* Kdk = MAC(Htotal,Z)    where Z is the DH of Pki and PKr */
    Zlen2 = sizeof(Z2);
    err = ECC_SharedSecret(eccPriv, eccPub, Z2, sizeof(Z2), &Zlen2);CKERR;
    
    OPTESTLogVerbose("\t\tECC Shared Secret (Z2):  (%ld bytes)\n",Zlen2);
    dumpHex(IF_LOG_DEBUG, Z2,  (int)Zlen2 , 0);
    OPTESTLogDebug("\n");
    
    OPTESTLogVerbose("\t\tCompare Secrets\n");
    err = compare2Results(Z1, Zlen1, Z2, Zlen2, kResultFormat_Byte, "ECC Shared Secret");CKERR;
    
    
done:
    if(eccPriv)
    {
        ECC_Free(eccPriv);
        eccPriv = kInvalidECC_ContextRef;
    }
    
    if(eccPub)
    {
        ECC_Free(eccPub);
        eccPub = kInvalidECC_ContextRef;
    }
    
    
    return err;
    
}


S4Err  TestECC()
{
    S4Err     err = kS4Err_NoErr;
    
    OPTESTLogInfo("\nTesting ECC\n");
    err = sTestECC(384); CKERR;
    err = sTestECC(414); CKERR;
    OPTESTLogInfo("\n");
    
    OPTESTLogInfo("Testing ECC-DH\n");
    err = sTestECC_DH(384); CKERR;
    OPTESTLogVerbose("\n");
    err = sTestECC_DH(414); CKERR;
    OPTESTLogInfo("\n");
    
    
    
    
done:
    return err;
    
}
