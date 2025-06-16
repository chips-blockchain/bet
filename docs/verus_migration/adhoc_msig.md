# Adhoc MultiSignatures
## Ad-Hoc Multisignature (msig) in Verus

Ad-hoc multisignature allows signing authorities to change over time, unlike traditional multisignatures where the signing authorities are fixed. This concept is first encountered when using VDXF IDs in Verus. In Verus, an ID is a human-readable name, and updates to it are controlled by primary addresses and the required minimum signatures. If the required minimum signatures exceed one, it essentially becomes a multisignature concept.

## Relevance in BET

In the BET ecosystem, multisignatures are extensively used to manage player funds. For each game, multisignatures are computed based on the availability of cashier nodes. Before a player can participate in any game, the system checks for available cashier nodes. Since cashier public key addresses are globally visible, the player uses the public keys of available cashiers to compute the `m-of-n` multisig address, where `m` is atleast `n/2 + 1`. This multisignature address changes as new cashiers join or existing cashiers leave. Therefore, there isn't a unique multisignature address that can be used consistently. However, with VDXF IDs, by keeping the ID name constant, the primary addresses that can update the ID and the number of signatures required can be modified. This flexibility allows for the removal of bad actors or the addition of new actors to the ecosystem without impacting end users.

