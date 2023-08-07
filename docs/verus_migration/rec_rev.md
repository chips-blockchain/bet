### Revocation and Recovery Authorities

Here we discuss the details about how we manage recovery and revocation authorities that has been used in bet ecosystem and also see some examples of how we creating them. 

So the question comes like do we need to use unique revoke/recovery authorities for every ID we maintain in the bet ecosystem or do we need to maintain one revoke/recovery authority for a group a ID's or do we use a single revoke/recovery authority for the entire bet ecosystem. Since the revoke/recovery authorities are viewed as separate identities so we can have a revoke/recovery authority which is controlled by multiple primary addresses.

We like to have more inputs on this to figure it out what could be the efficient way. 

For now we see how a revoke/recovery authority is created and use them in registering the other IDs. The following commands are executed on the `chips10sec` test chain.

#### Registering the revoke_2 ID
```
#verus -chain=chips10sec registernamecommitment revoke_2 RV84ZKrfNiTXXCJG7HouWyFMa229WSSKoE "" "poker.chips10sec"
{
  "txid": "e8728b60f2b8ec617173aa7f34e322de24140c54122d1f9abfc00cbd33a41e2b",
  "namereservation": {
    "version": 1,
    "name": "revoke_2",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "salt": "37a7b6a94946e81cf834dad8817d7afcbc966a426c7d099fcc70cdd0de5580ff",
    "referral": "",
    "nameid": "iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX"
  }
}

#verus -chain=chips10sec registeridentity '{
  "txid": "e8728b60f2b8ec617173aa7f34e322de24140c54122d1f9abfc00cbd33a41e2b",
  "namereservation": {
    "version": 1,
    "name": "revoke_2",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "salt": "37a7b6a94946e81cf834dad8817d7afcbc966a426c7d099fcc70cdd0de5580ff",
    "referral": "",
    "nameid": "iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX"
  }, 
    "identity":{
        "name":"revoke_2.poker.chips10sec", 
        "primaryaddresses":["RV84ZKrfNiTXXCJG7HouWyFMa229WSSKoE"], 
	    "minimumsignatures":1, 
        "privateaddress": ""
    }
}'
a393edc16f433046dad20dd3e7ef82d85db86d41fc257d49ecf411a6d3dce36a
```

#### Registering the recovery_2 ID

```
#verus -chain=chips10sec registernamecommitment recovery_2 RVvsdGFt1rgqRTqCdtRqNgy2JUFWNyfCcs "" "poker.chips10sec"
{
  "txid": "c6b0a55343f6fa5c88f6f2e248f989478c44355dae9bc9a47fa40bcff7352fe8",
  "namereservation": {
    "version": 1,
    "name": "recovery_2",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "salt": "a970f098571f91b431dde2478dd6b7450f92bb4766c4f736497da7f24cd4aed3",
    "referral": "",
    "nameid": "iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj"
  }
}

#verus -chain=chips10sec registeridentity '{
  "txid": "c6b0a55343f6fa5c88f6f2e248f989478c44355dae9bc9a47fa40bcff7352fe8",
  "namereservation": {
    "version": 1,
    "name": "recovery_2",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "salt": "a970f098571f91b431dde2478dd6b7450f92bb4766c4f736497da7f24cd4aed3",
    "referral": "",
    "nameid": "iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj"
  }, 
    "identity":{
        "name":"recovery_2.poker.chips10sec", 
        "primaryaddresses":["RVvsdGFt1rgqRTqCdtRqNgy2JUFWNyfCcs"], 
	    "minimumsignatures":1, 
        "privateaddress": ""
    }
}'
```

#### Registering test_6 ID using revoke_2 and recovery_2 as revoke and recovery authorities

```
#verus -chain=chips10sec registernamecommitment test_6 RFtKjKRisud8dGvbuJxbrtS1NZDKT5FLQu "" "poker.chips10sec"
{
  "txid": "b9b9df30c178068b59a4f5904cbe4dd27e70fecbe177e257b063c5aabfd311d8",
  "namereservation": {
    "version": 1,
    "name": "test_6",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "salt": "a85cc6c330f4ee5a904b30e3285875b89b1d151068f087189c0618cf98f95c33",
    "referral": "",
    "nameid": "i7otjcRubuStcdhE99nYZAtfpzwnxQtQ4z"
  }
}

#verus -chain=chips10sec registeridentity '{
  "txid": "b9b9df30c178068b59a4f5904cbe4dd27e70fecbe177e257b063c5aabfd311d8",
  "namereservation": {
    "version": 1,
    "name": "test_6",
    "parent": "i6gViGxt7YinkJZoubKdbWBrqdRCb1Rkvs",
    "salt": "a85cc6c330f4ee5a904b30e3285875b89b1d151068f087189c0618cf98f95c33",
    "referral": "",
    "nameid": "i7otjcRubuStcdhE99nYZAtfpzwnxQtQ4z"
  }, 
    "identity":{
        "name":"test_6", 
        "primaryaddresses":["RFtKjKRisud8dGvbuJxbrtS1NZDKT5FLQu"], 
		"revocationauthority":"iSCt7uQBePbTSJUSPAuQqv3Qjw1YZmj6FX", 
		"recoveryauthority":"iGXhgDHN7GBmbPPXcvNoj4Lc99pQEoA8Fj",
	    "minimumsignatures":1, 
        "privateaddress": ""
    }
}'
3f79202e298eeb37f93565edd8bdf02d264e6415ecb8126f1116cc4af8bbb41b
```
