<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile>
  <compound kind="file">
    <name>bet.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>dd/d53/bet_8c</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d0/db9/cards777_8h" name="cards777.h" local="yes" imported="no">cards777.h</includes>
    <includes id="d5/d3a/cashier_8h" name="cashier.h" local="yes" imported="no">cashier.h</includes>
    <includes id="d8/de1/client_8h" name="client.h" local="yes" imported="no">client.h</includes>
    <includes id="d5/d90/commands_8h" name="commands.h" local="yes" imported="no">commands.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d4/d71/gfshare_8h" name="gfshare.h" local="yes" imported="no">gfshare.h</includes>
    <includes id="df/ded/host_8h" name="host.h" local="yes" imported="no">host.h</includes>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <includes id="dd/d98/table_8h" name="table.h" local="yes" imported="no">table.h</includes>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bet_player_initialize</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a1fb0766f6a7a21ce0bf73b4a3cbb4a02</anchor>
      <arglist>(char *host_ip, const int32_t port)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bet_bvv_initialize</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>abb41d56704e5ba1aea3c7c97e67cee9a</anchor>
      <arglist>(char *host_ip, const int32_t port)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bet_dcv_initialize</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>aaee466e723c622115fa30a49a699e97e</anchor>
      <arglist>(char *host_ip, const int32_t port)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>main</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a3c04138a5bfe5d72780bb7e82a18e627</anchor>
      <arglist>(int argc, char **argv)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>curve25519_fieldelement</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a486c8d59b898af86c8e498798a3d500a</anchor>
      <arglist>(bits256 hash)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>card_rand256</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a639f0e80f94d1ce9adf7bb0d52f8f77b</anchor>
      <arglist>(int32_t privkeyflag, int8_t index)</arglist>
    </member>
    <member kind="function">
      <type>struct pair256</type>
      <name>deckgen_common</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>af1caf4bd39a77add770c8e9a9b859c25</anchor>
      <arglist>(struct pair256 *randcards, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>deckgen_common2</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a15b788badb600230760e48274d4f36b6</anchor>
      <arglist>(struct pair256 *randcards, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>struct pair256</type>
      <name>deckgen_player</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>ae418903f2f00fb0f13c7d21858a4efdd</anchor>
      <arglist>(bits256 *playerprivs, bits256 *playercards, int32_t *permis, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>sg777_deckgen_vendor</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a8d60448109ceb768b85e3e46a56b523d</anchor>
      <arglist>(int32_t playerid, bits256 *cardprods, bits256 *finalcards, int32_t numcards, bits256 *playercards, bits256 deckid)</arglist>
    </member>
    <member kind="function">
      <type>struct pair256</type>
      <name>p2p_bvv_init</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>aed0d6ecdf0b295306ebdc2ce145bdd01</anchor>
      <arglist>(bits256 *keys, struct pair256 b_key, bits256 *blindings, bits256 *blindedcards, bits256 *finalcards, int32_t numcards, int32_t numplayers, int32_t playerid, bits256 deckid)</arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>bet_player</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>acce96a8256760a7d9ee28b0d0b722e29</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_vars *</type>
      <name>player_vars</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>abe20de78a27f415da1e9437e26fd2061</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>sharenrs</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>ad0579cac3407f0f0cb742891f36d6e56</anchor>
      <arglist>[256]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>deckid</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>acab83b0ae8f9771b03b3a9a1d51eeffe</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>playershares</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a4f81cce59752f55ccee614846f8dfec8</anchor>
      <arglist>[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis_d</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a68a976bb37813f4fa860943eecc2779a</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis_b</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a42ef3c7ba5e18cd42816bcb7def71fbf</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>v_hash</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>ad6edd7e1c4d864d80357255a938dd8b9</anchor>
      <arglist>[CARDS777_MAXCARDS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>g_hash</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a0d28177dced0059c86fb2c7fc9c8d660</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>struct enc_share *</type>
      <name>g_shares</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a9af9f2fdf27f0f54f624d89b32af82ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>max_players</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a097028f2c20ab139a38e5c621193f223</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const int32_t</type>
      <name>poker_deck_size</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a72cf71d5ef0640767677fd54d4729359</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const char *</type>
      <name>rootAddress</name>
      <anchorfile>dd/d53/bet_8c.html</anchorfile>
      <anchor>a538351e3178156bc6cc8e6156c6d88ba</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>bet.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>df/d5a/bet_8h</filename>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <class kind="struct">BET_shardsinfo</class>
    <class kind="struct">gfshare_ctx_bet</class>
    <class kind="struct">privatebet_info</class>
    <class kind="struct">privatebet_rawpeerln</class>
    <class kind="struct">privatebet_peerln</class>
    <class kind="struct">privatebet_vars</class>
    <class kind="struct">pair256</class>
    <class kind="struct">privatebet_share</class>
    <class kind="struct">enc_share</class>
    <class kind="struct">deck_player_info</class>
    <class kind="struct">deck_dcv_info</class>
    <class kind="struct">deck_bvv_info</class>
    <class kind="struct">cashier</class>
    <member kind="define">
      <type>#define</type>
      <name>PORT</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a614217d263be1fb1a5f76e2ff7be19a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>action_type</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>small_blind</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687a9068486ed14b88864e4ddbe5c42d3519</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>big_blind</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687aaaa621c44e114e3e22f2c23f3d62dcf8</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>check</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687a8961e41d90f82b7cf2573e175337722a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>raise</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687ab13f5aed8ee81ec8a73046c062c33dac</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>call</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687a98ab4979bd774d191f9a4642f9df392f</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>allin</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687a0d155be11acdd7b79514830c4db72559</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>fold</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a864de25b94a13d8c17e790b2e1638687a8a644c686eb987d3a5d2b55bae4c9a3a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumeration">
      <type></type>
      <name>card_type</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>burn_card</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14a58562e432b5f58ab1c4eb0e13166f201</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>hole_card</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14a4969c2cbaa32a74db09d8559d1c0b1a9</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>flop_card_1</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14a02ead816385097389854682941be41e2</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>flop_card_2</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14ad2cdc62f85d20ec1fe77d3e49b044f20</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>flop_card_3</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14a7cf51dfa037d510b321774807d19ab04</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>turn_card</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14a317688c06860216584fd68751241a00a</anchor>
      <arglist></arglist>
    </member>
    <member kind="enumvalue">
      <name>river_card</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aee707e2ff5691dbdefd9341a18de7b14a89c9fed6b7d99172a68a2e4890393b0b</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>xoverz_donna</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>ac1717706afec7e9fd7ce46acc957a381</anchor>
      <arglist>(bits256 a)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>crecip_donna</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a252e0af563e434c78a3a62a3246ca976</anchor>
      <arglist>(bits256 a)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>fmul_donna</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a4eed07cb5561b32c853dd9ac514ea68b</anchor>
      <arglist>(bits256 a, bits256 b)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>card_rand256</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a639f0e80f94d1ce9adf7bb0d52f8f77b</anchor>
      <arglist>(int32_t privkeyflag, int8_t index)</arglist>
    </member>
    <member kind="function">
      <type>struct pair256</type>
      <name>deckgen_common</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>af1caf4bd39a77add770c8e9a9b859c25</anchor>
      <arglist>(struct pair256 *randcards, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>struct pair256</type>
      <name>deckgen_player</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>ae418903f2f00fb0f13c7d21858a4efdd</anchor>
      <arglist>(bits256 *playerprivs, bits256 *playercards, int32_t *permis, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>sg777_deckgen_vendor</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a8d60448109ceb768b85e3e46a56b523d</anchor>
      <arglist>(int32_t playerid, bits256 *cardprods, bits256 *finalcards, int32_t numcards, bits256 *playercards, bits256 deckid)</arglist>
    </member>
    <member kind="function">
      <type>struct pair256</type>
      <name>p2p_bvv_init</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>aed0d6ecdf0b295306ebdc2ce145bdd01</anchor>
      <arglist>(bits256 *keys, struct pair256 b_key, bits256 *blindings, bits256 *blindedcards, bits256 *finalcards, int32_t numcards, int32_t numplayers, int32_t playerid, bits256 deckid)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>curve25519_fieldelement</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a486c8d59b898af86c8e498798a3d500a</anchor>
      <arglist>(bits256 hash)</arglist>
    </member>
    <member kind="variable">
      <type>struct enc_share *</type>
      <name>g_shares</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a9af9f2fdf27f0f54f624d89b32af82ff</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct enc_share *</type>
      <name>all_g_shares</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>a26137bbeb0831e55a2a1207feb76264e</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>bet_player</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>acce96a8256760a7d9ee28b0d0b722e29</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_vars *</type>
      <name>player_vars</name>
      <anchorfile>df/d5a/bet_8h.html</anchorfile>
      <anchor>abe20de78a27f415da1e9437e26fd2061</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>cards777.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>d5/d08/cards777_8c</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d4/d71/gfshare_8h" name="gfshare.h" local="yes" imported="no">gfshare.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_permutation</name>
      <anchorfile>d5/d08/cards777_8c.html</anchorfile>
      <anchor>a8702768abbdbd84504aed93cde7a5bc8</anchor>
      <arglist>(int32_t *permi, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_cipher_create</name>
      <anchorfile>d5/d08/cards777_8c.html</anchorfile>
      <anchor>a046144d8354943e2b9531ec729e7d363</anchor>
      <arglist>(bits256 privkey, bits256 destpub, uint8_t *cipher, uint8_t *data, int32_t datalen)</arglist>
    </member>
    <member kind="function">
      <type>uint8_t *</type>
      <name>bet_decrypt</name>
      <anchorfile>d5/d08/cards777_8c.html</anchorfile>
      <anchor>a0e224df65f933e461479b2eeaa0976a4</anchor>
      <arglist>(uint8_t *decoded, int32_t maxsize, bits256 senderpub, bits256 mypriv, uint8_t *ptr, int32_t *recvlenp)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>cards777.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d0/db9/cards777_8h</filename>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_permutation</name>
      <anchorfile>d0/db9/cards777_8h.html</anchorfile>
      <anchor>a8702768abbdbd84504aed93cde7a5bc8</anchor>
      <arglist>(int32_t *permi, int32_t numcards)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_cipher_create</name>
      <anchorfile>d0/db9/cards777_8h.html</anchorfile>
      <anchor>a046144d8354943e2b9531ec729e7d363</anchor>
      <arglist>(bits256 privkey, bits256 destpub, uint8_t *cipher, uint8_t *data, int32_t datalen)</arglist>
    </member>
    <member kind="function">
      <type>uint8_t *</type>
      <name>bet_decrypt</name>
      <anchorfile>d0/db9/cards777_8h.html</anchorfile>
      <anchor>a0e224df65f933e461479b2eeaa0976a4</anchor>
      <arglist>(uint8_t *decoded, int32_t maxsize, bits256 senderpub, bits256 mypriv, uint8_t *ptr, int32_t *recvlenp)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>cashier.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>de/d4f/cashier_8c</filename>
    <member kind="variable">
      <type>struct cashier *</type>
      <name>cashier_info</name>
      <anchorfile>de/d4f/cashier_8c.html</anchorfile>
      <anchor>a28b711b1a0ece6b40a95e022e13c7425</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>cashier.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d5/d3a/cashier_8h</filename>
    <member kind="function">
      <type>void</type>
      <name>bet_cashier_loop</name>
      <anchorfile>d5/d3a/cashier_8h.html</anchorfile>
      <anchor>a58481d6501f0921ac04c5e279dd52a5d</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>client.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>dd/d93/client_8c</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d0/db9/cards777_8h" name="cards777.h" local="yes" imported="no">cards777.h</includes>
    <includes id="d8/de1/client_8h" name="client.h" local="yes" imported="no">client.h</includes>
    <includes id="d5/d90/commands_8h" name="commands.h" local="yes" imported="no">commands.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d4/d71/gfshare_8h" name="gfshare.h" local="yes" imported="no">gfshare.h</includes>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <includes id="d1/d8d/payment_8h" name="payment.h" local="yes" imported="no">payment.h</includes>
    <includes id="db/ddb/states_8h" name="states.h" local="yes" imported="no">states.h</includes>
    <includes id="dd/d98/table_8h" name="table.h" local="yes" imported="no">table.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>_POSIX_C_SOURCE</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a3024ccd4a9af5109d24e6c57565d74a1</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LWS_PLUGIN_STATIC</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a6ac88fa6a0ff7931eebef9c3e789b53d</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>player_lws_write</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a36821c272564eee5edb1ffa9b343f507</anchor>
      <arglist>(cJSON *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>make_command</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad339ee7b7e00b3442d3427399c68d9ae</anchor>
      <arglist>(int argc, char **argv, cJSON **argjson)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>enc_share_str</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ac924306e35b6acc0397ce7698f6e035a</anchor>
      <arglist>(char hexstr[177], struct enc_share x)</arglist>
    </member>
    <member kind="function">
      <type>struct enc_share</type>
      <name>get_API_enc_share</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af5019469cb33ffe870230d4fc1dbc17e</anchor>
      <arglist>(cJSON *obj)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_init</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a02f69144de0f51d065fe1722db95c0fd</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_bvv_join_init</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a34abc6db860e03c1ac45e4a0cc60c75a</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_connect</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ae4364bdbf746f8389d371c6036a6b545</anchor>
      <arglist>(char *uri)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static cJSON *</type>
      <name>bet_player_fundchannel</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a5e0edf5f933f02de44629e3c0e502ec1</anchor>
      <arglist>(char *channel_id)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_check_bvv_ready</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a4ad2af1820b2b0e342828218f23f320f</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_bvv_reset</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad27c1576d4f71b16c47d1da6291b9b3e</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_frontend</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ae3fb41c147fb293463b03949fb184d33</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_bvv_backend_loop</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a197bd07bf82a94eee3dcdb8d0ded9328</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_live_response</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>adcf149d4c18c9fe19e76d4700de8ec46</anchor>
      <arglist>(struct privatebet_info *bet, char *node_type, int32_t playerid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_backend</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a5867edd57ad831ca8ce5c1d5be8340ad</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>bet_decode_card</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a58c2f54c3708d866c6de95b89c8fa640</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int32_t cardid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_pay_invoice</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aae312929e00d18a0b4b799beb8351aa1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_player_betting_invoice</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aad0c032fd04f6e8a2398273c1b438c96</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_player_winner</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af7ba085f6f1a0fd81f0ec7eff12faa15</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_player_bet_round</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad3ea7038053ebf89ac283e9cd8c69930</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>display_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a0e893535965862383d61dc592d60de6f</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_receive_share</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad6f7d36feece182dc2ac96e835a1f351</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_player_ask_share</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ae41060baf8bf1f8a80b49ddbfb85d334</anchor>
      <arglist>(struct privatebet_info *bet, int32_t cardid, int32_t playerid, int32_t card_type)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_give_share</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad1ad69c2023ff6b03b2c4b5c5a1fa296</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_get_own_share</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad17fc99fd42e30c571716a302724f681</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_turn</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a4b45c2e90c46014b31d84ce3ee77c88d</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_ready</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ada27213bbd6fb01eb48380cf6a545f50</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_bvv_init</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aa3efbe93a024418bad4fb0c7dd36c948</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_dcv_init</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a8bc0b96547a49db928b8b2bb9decdea9</anchor>
      <arglist>(cJSON *dcv_info, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_init</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ab3f322b02eb93a31065197739f17f97d</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ln_connect</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad97b26b7f7cb8263aacafa96e036dd21</anchor>
      <arglist>(char *id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ln_check_peer_and_connect</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a3640ff6ffbb91526afe25117ed5d2f69</anchor>
      <arglist>(char *id)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_get_channel_status</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad27a4916bafd17c35c6d415131471118</anchor>
      <arglist>(char *id)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_find_channel_balance</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aa8658785cefc47b902f9739f4c057ecd</anchor>
      <arglist>(char *uri)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_check_player_stack</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af0c4b735deff7caa8fdb0690f5fcf475</anchor>
      <arglist>(char *uri)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_join_res</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a5ae38585cfd0bbdada6c474b3c75287b</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_join</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aa4cffb9530c42dda5b4f9a5a71a40d91</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_table_info</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>afc59069ea715bb2dbddac0566980267e</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_reset</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a3fb055473da5e11979bc06373f0eb8cb</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_frontend</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a5f607e543bbd3f25bd131fd16dbde74b</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>lws_callback_http_player</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a8561c0adf5cd4cd42bfa89efea81400d</anchor>
      <arglist>(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>player_sigint_handler</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aae1d74e648774b5ec5a4e94f57439d2e</anchor>
      <arglist>(int sig)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_player_frontend_loop</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a392754f34bd754272a4557a1d01e1de5</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>lws_callback_http_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a697a59031818f0880cfb1395f6161016</anchor>
      <arglist>(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_bvv_frontend_loop</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af0c08e23d10002f78cc2012d59dd7ac8</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_push_client</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a78e953bcd3956ce5d9ef3dc35d2eb479</anchor>
      <arglist>(cJSON *argjson)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bet_player_blinds_info</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aafa872bf2874caf5e37161bb1f74f053</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bet_push_join_info</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af82402c98d010160f34983391f13c995</anchor>
      <arglist>(int32_t playerid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_backend</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aabdfb1c57161b946bd917a76896b4ce7</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_player_backend_loop</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a63453d2ce76fe58775a3c25bef598f6a</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_listfunds</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a34f717d66716083a33db186556c1b503</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_get_uri</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>adfabee42ac4edb8af80610215ee2f96f</anchor>
      <arglist>(char **uri)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>bet_get_deckid</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ae96e125f8a557913057f5a9ba24422af</anchor>
      <arglist>(int32_t playerID)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_fundChannel</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ab4c0283de7c6c864ba138f812bcb42c8</anchor>
      <arglist>(char *channel_id)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_connect_uri</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a568e321cab9e12012475b221d8530454</anchor>
      <arglist>(char *uri)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>rest_push_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a538ef7629127f61c06768b0c82bd5d75</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson, int32_t this_playerID)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>rest_display_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>abd9f55325988d1892357a6d96d5f0348</anchor>
      <arglist>(cJSON *argjson, int32_t this_playerID)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_pay</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a680eb239a111a0e59f5310c8bbb47c51</anchor>
      <arglist>(char *bolt11)</arglist>
    </member>
    <member kind="variable">
      <type>struct lws *</type>
      <name>wsi_global_client</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af76d3fd39038b5f384027b8ecf35298e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>ws_connection_status</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a41d84e757e4268b649447ec9b028e50b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct lws *</type>
      <name>wsi_global_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a084089c1850bb22003e30a78ea591d4a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_card_matrix</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a2b55495f54ee4b8903b54e820f54cdbc</anchor>
      <arglist>[hand_size]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_card_values</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ab2112ae5fc96d04af95803ae923b07d5</anchor>
      <arglist>[hand_size]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>number_cards_drawn</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aeb04c3fa1165fa5fa9bae1baf193ca57</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>sharesflag</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a1b8957847884792ed319bcbd84fae5cd</anchor>
      <arglist>[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>data_exists</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a68c1b0453bac046ef727e0fb7ae6ef07</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>player_gui_data</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a5677354499ab197cb2d62ae94ac05e31</anchor>
      <arglist>[1024]</arglist>
    </member>
    <member kind="variable">
      <type>struct deck_player_info</type>
      <name>player_info</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a0038a05fcd3129996b6a2c2ec7c35fd2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct deck_bvv_info</type>
      <name>bvv_info</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aa4ba82dfd9f0dc7b5fd0356282d246fa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_shares</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a6bd1a5acb5b0fdfea91c13cd272593ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aa2afd0641274b706d1d95a9529320ee2</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_player_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ab0528ad388ed4337ba5caa5549d4147c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_id</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a8d52f2505067e478ebb3c854ab8d2e35</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_joined</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ae8f5037be8051f3514e98d1749b712d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>all_v_hash</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a08c12bf86b3d4a32047df51eb6c3bf2f</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>all_g_hash</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>adbe5b279a5f26562f03258cd5c3b57f9</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_sharesflag</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a135c13470f475c2094e6cb1e03c1f87a</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_player_card_values</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aedd00ee3a421afe12a3fafb9c2bca8b6</anchor>
      <arglist>[CARDS777_MAXPLAYERS][hand_size]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_number_cards_drawn</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a98df73dcf4c0700e5f9953e6abf0aeba</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_player_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a70f8b98912f3ec905230bb8fd4309f63</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_no_of_player_cards</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a7bbc5875989acbad854ce8792d7c1e07</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>all_playershares</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a17dcd5f3ed168c574c56b6f9db2076be</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>struct enc_share *</type>
      <name>all_g_shares</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a26137bbeb0831e55a2a1207feb76264e</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>bet_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a970c263064aa5ae0af28ee68ca0e88e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_vars *</type>
      <name>bvv_vars</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ac49dcdcdce0afba5b6d3966d1ee9aa2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>BET_player</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a197d129756de1b75ac59948666af6d60</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>struct deck_player_info</type>
      <name>all_players_info</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ad9407847dd8bb4a74f8c1a690281bf4d</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>lws_buf_1</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a02e14a3d380cdc62eb09e458488c1e36</anchor>
      <arglist>[65536]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>lws_buf_length_1</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a24e383e25a68fc20c7ac5c2244728933</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>lws_buf_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a230d70f0c3046d76fe0d8a7da97b11fb</anchor>
      <arglist>[2000]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>lws_buf_length_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aa075c466195ca84dfa726f2d53195d03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static struct lws_protocols</type>
      <name>player_http_protocol</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a04f41b81d9c7b568e12088ef8ad97364</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>interrupted1</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>aeff0e8010679bb81070f86a4bd09d326</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const struct lws_http_mount</type>
      <name>lws_http_mount_player</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a34646bfc439d4d74de5788b2788f6c8f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static struct lws_protocols</type>
      <name>protocols_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>af71d733baab9d1da4b27f7c2dcc481b4</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>interrupted_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>a0293501df1c31026c628d50d14ff4291</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const struct lws_http_mount</type>
      <name>mount_bvv</name>
      <anchorfile>dd/d93/client_8c.html</anchorfile>
      <anchor>ab686b2af0d26cfb26c183069ec3274cc</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>client.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d8/de1/client_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>char *</type>
      <name>enc_share_str</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ac924306e35b6acc0397ce7698f6e035a</anchor>
      <arglist>(char hexstr[177], struct enc_share x)</arglist>
    </member>
    <member kind="function">
      <type>struct enc_share</type>
      <name>get_API_enc_share</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>af5019469cb33ffe870230d4fc1dbc17e</anchor>
      <arglist>(cJSON *obj)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_init</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a02f69144de0f51d065fe1722db95c0fd</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_backend</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a5867edd57ad831ca8ce5c1d5be8340ad</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_bvv_reset</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad27c1576d4f71b16c47d1da6291b9b3e</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>bet_decode_card</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a58c2f54c3708d866c6de95b89c8fa640</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int32_t cardid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_receive_share</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad6f7d36feece182dc2ac96e835a1f351</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_give_share</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad1ad69c2023ff6b03b2c4b5c5a1fa296</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_get_own_share</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad17fc99fd42e30c571716a302724f681</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_turn</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a4b45c2e90c46014b31d84ce3ee77c88d</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_bvv_init</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>aa3efbe93a024418bad4fb0c7dd36c948</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_dcv_init</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a8bc0b96547a49db928b8b2bb9decdea9</anchor>
      <arglist>(cJSON *dcv_info, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_init</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ab3f322b02eb93a31065197739f17f97d</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_join_res</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a5ae38585cfd0bbdada6c474b3c75287b</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_client_join</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>aa4cffb9530c42dda5b4f9a5a71a40d91</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_player_backend_loop</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a63453d2ce76fe58775a3c25bef598f6a</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_get_channel_status</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad27a4916bafd17c35c6d415131471118</anchor>
      <arglist>(char *id)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_ready</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ada27213bbd6fb01eb48380cf6a545f50</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_table_info</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>afc59069ea715bb2dbddac0566980267e</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>display_cards</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a0e893535965862383d61dc592d60de6f</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_reset</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a3fb055473da5e11979bc06373f0eb8cb</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_player_frontend_loop</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>af0d149e747ee4195f686b9093a77309a</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_backend</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>aabdfb1c57161b946bd917a76896b4ce7</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_frontend</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a5f607e543bbd3f25bd131fd16dbde74b</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_bvv_backend_loop</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a197bd07bf82a94eee3dcdb8d0ded9328</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_bvv_frontend</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ae3fb41c147fb293463b03949fb184d33</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_bvv_frontend_loop</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>af0c08e23d10002f78cc2012d59dd7ac8</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>bits256</type>
      <name>bet_get_deckid</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ae96e125f8a557913057f5a9ba24422af</anchor>
      <arglist>(int32_t playerID)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_push_client</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a78e953bcd3956ce5d9ef3dc35d2eb479</anchor>
      <arglist>(cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>make_command</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad339ee7b7e00b3442d3427399c68d9ae</anchor>
      <arglist>(int argc, char **argv, cJSON **argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_listfunds</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a34f717d66716083a33db186556c1b503</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_get_uri</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>adfabee42ac4edb8af80610215ee2f96f</anchor>
      <arglist>(char **uri)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_connect_uri</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a568e321cab9e12012475b221d8530454</anchor>
      <arglist>(char *uri)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_fundChannel</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ab4c0283de7c6c864ba138f812bcb42c8</anchor>
      <arglist>(char *channel_id)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_pay</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a680eb239a111a0e59f5310c8bbb47c51</anchor>
      <arglist>(char *bolt11)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ln_connect</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>ad97b26b7f7cb8263aacafa96e036dd21</anchor>
      <arglist>(char *id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>ln_check_peer_and_connect</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a3640ff6ffbb91526afe25117ed5d2f69</anchor>
      <arglist>(char *id)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>rest_push_cards</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a538ef7629127f61c06768b0c82bd5d75</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson, int32_t this_playerID)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_pay_invoice</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>aae312929e00d18a0b4b799beb8351aa1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>player_lws_write</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>a36821c272564eee5edb1ffa9b343f507</anchor>
      <arglist>(cJSON *data)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>rest_display_cards</name>
      <anchorfile>d8/de1/client_8h.html</anchorfile>
      <anchor>abd9f55325988d1892357a6d96d5f0348</anchor>
      <arglist>(cJSON *argjson, int32_t this_playerID)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>commands.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>de/d21/commands_8c</filename>
    <includes id="d5/d90/commands_8h" name="commands.h" local="yes" imported="no">commands.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d8/de1/client_8h" name="client.h" local="yes" imported="no">client.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d1/d48/oracle_8h" name="oracle.h" local="yes" imported="no">oracle.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_iswatchonly</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a739d7fb76e0f339cfd05fcd3af1a7996</anchor>
      <arglist>(char *address)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_spend_multi_sig_address</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a02d0209f0f84414814db784e6789fa8a</anchor>
      <arglist>(char *address, double amount)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_import_address</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a30b8a6f31c7cff766088550d724b65e7</anchor>
      <arglist>(char *address)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>chips_get_new_address</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a22a4ef884a7340e5b5942b6d07f6324c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>chips_validate_address</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>ae809d04ee9f63dc499c935b837533281</anchor>
      <arglist>(char *address)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_list_address_groupings</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>afae8e5b50da7091be7b91d5e112c25ee</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_transfer_funds</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>aa2f5924aa180ba7f7340d699a6f77027</anchor>
      <arglist>(double amount, char *address)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_send_raw_tx</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a12e5cfa7bc2b417750b9b3394f93f95a</anchor>
      <arglist>(cJSON *signedTransaction)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_sign_raw_tx_with_wallet</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a651c6b1b8725fb048acb5cf321dc249a</anchor>
      <arglist>(char *rawtransaction)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_publish_multisig_tx</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a0a14a4f38a09aa381473cbd43357582d</anchor>
      <arglist>(char *tx)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_create_raw_multi_sig_tx</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a127e79ac6db1ec54d970d46b0c24516d</anchor>
      <arglist>(double amount, char *toaddress, char *fromaddress)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_create_raw_tx</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>accffaa855b618cd9a7e274a3ea3d3e38</anchor>
      <arglist>(double amount, char *address)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_list_unspent</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>adbd1855ec50eecc1c799ef803d64f202</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_get_block_count</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>aa536c2d22ecb79268d31956d7ec4f36d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_dev_block_height</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>aa9981a010ee441f6ecd10019f90eb674</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_ln_chips_sync</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>af32277231a4357cbc4743ce3f8361fc9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>chips_get_balance</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a17c5067dbb627fd8f2eca1461a012e03</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_lock_transaction</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a8f35ff6c18f2b0a991c3fc583615c2b0</anchor>
      <arglist>(int32_t fundAmount)</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>multisigAddress</name>
      <anchorfile>de/d21/commands_8c.html</anchorfile>
      <anchor>a5ec7d8bca65eaaa8846d79ad209d9cd7</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>commands.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d5/d90/commands_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_iswatchonly</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a739d7fb76e0f339cfd05fcd3af1a7996</anchor>
      <arglist>(char *address)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_spend_multi_sig_address</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a02d0209f0f84414814db784e6789fa8a</anchor>
      <arglist>(char *address, double amount)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_import_address</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a30b8a6f31c7cff766088550d724b65e7</anchor>
      <arglist>(char *address)</arglist>
    </member>
    <member kind="function">
      <type>char *</type>
      <name>chips_get_new_address</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a22a4ef884a7340e5b5942b6d07f6324c</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>chips_validate_address</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>ae809d04ee9f63dc499c935b837533281</anchor>
      <arglist>(char *address)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_list_address_groupings</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>afae8e5b50da7091be7b91d5e112c25ee</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_transfer_funds</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>aa2f5924aa180ba7f7340d699a6f77027</anchor>
      <arglist>(double amount, char *address)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_send_raw_tx</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a12e5cfa7bc2b417750b9b3394f93f95a</anchor>
      <arglist>(cJSON *signedTransaction)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_sign_raw_tx_with_wallet</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a651c6b1b8725fb048acb5cf321dc249a</anchor>
      <arglist>(char *rawtransaction)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_publish_multisig_tx</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a0a14a4f38a09aa381473cbd43357582d</anchor>
      <arglist>(char *tx)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_create_raw_multi_sig_tx</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a127e79ac6db1ec54d970d46b0c24516d</anchor>
      <arglist>(double amount, char *toaddress, char *fromaddress)</arglist>
    </member>
    <member kind="function">
      <type>cJSON *</type>
      <name>chips_create_raw_tx</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>accffaa855b618cd9a7e274a3ea3d3e38</anchor>
      <arglist>(double amount, char *address)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>chips_list_unspent</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>adbd1855ec50eecc1c799ef803d64f202</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_get_block_count</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>aa536c2d22ecb79268d31956d7ec4f36d</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>ln_dev_block_height</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>aa9981a010ee441f6ecd10019f90eb674</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>check_ln_chips_sync</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>af32277231a4357cbc4743ce3f8361fc9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>double</type>
      <name>chips_get_balance</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a17c5067dbb627fd8f2eca1461a012e03</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>chips_lock_transaction</name>
      <anchorfile>d5/d90/commands_8h.html</anchorfile>
      <anchor>a8f35ff6c18f2b0a991c3fc583615c2b0</anchor>
      <arglist>(int32_t fundAmount)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>common.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>dc/d54/common_8h</filename>
    <member kind="define">
      <type>#define</type>
      <name>hand_size</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>abf1aff3f2c475b56db3259d85f51376d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>no_of_hole_cards</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a74e3d4c9ddfaa1e6305992c604ba5059</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>no_of_flop_cards</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a1b178fe4242f4486d08c06e88043fb01</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>no_of_turn_card</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a6e9f7ad39c80989698e502de2672464d</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>no_of_river_card</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a748aa14f4f41d8a0da358dca6ae683c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>no_of_community_cards</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a69d7d5479f5a621b1b23d6fa902a9a92</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NSUITS</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a8f71a68d7f10d41f9c251dee9afd3c57</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>NFACES</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a7a60859a9235a326a6fd937fd0d5a578</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>small_blind_amount</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ac4c919d9b585cfb0f9efebde8ad122a2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>big_blind_amount</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>af8068d216af6037c357dd0b16bd83968</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>table_stack</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ad2e4522eaaa8ea16ddbdcd20db27c6ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>mchips_msatoshichips</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a004597c264a6803f5882ee59cf86c3d6</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>channel_fund_satoshis</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ab2411585d63a643fa0ca1dd28d709724</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CARDS777_MAXCARDS</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a90f0ea2d767ac1915e44acb24a1fe1bd</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CARDS777_MAXPLAYERS</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a97d6188fd175f4ea7268e9445107a7c0</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CARDS777_MAXROUNDS</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>aa7cb3dc82286cdcfc50250b3ec3ae769</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CARDS777_MAXCHIPS</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ac883e19e289bb9e33d071c975d985155</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>CARDS777_CHIPSIZE</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a625c5123ed07e8da23de476daee01997</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BET_PLAYERTIMEOUT</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a276bc74404b4f89419a12d65fa9a86d7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BET_GAMESTART_DELAY</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>afcab9c9548a0a2caaf0a44d372da0326</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>BET_RESERVERATE</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ac4eec8ce78a9dddee1101f949bfa95c7</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>LN_FUNDINGERROR</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>add535efbc013b90981a8232daf9407ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>v_hash</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ad6edd7e1c4d864d80357255a938dd8b9</anchor>
      <arglist>[CARDS777_MAXCARDS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>g_hash</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a0d28177dced0059c86fb2c7fc9c8d660</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>all_v_hash</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a08c12bf86b3d4a32047df51eb6c3bf2f</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>all_g_hash</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>adbe5b279a5f26562f03258cd5c3b57f9</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>bet_dcv</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a62b702b575af739ee2c999554ea33c5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_vars *</type>
      <name>dcv_vars</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ab98cf25f28f49f78a9431fcc11bda58e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_signers</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a79d6b450d204447564e6deee2bc97d82</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>max_no_of_signers</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>afc2fd24df5d82cbedddde3b28245f46a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>is_signed</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ae56653403779513733fee433ef0b6aab</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>bet_bvv</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a970c263064aa5ae0af28ee68ca0e88e7</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_vars *</type>
      <name>bvv_vars</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ac49dcdcdce0afba5b6d3966d1ee9aa2a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>BET_player</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a197d129756de1b75ac59948666af6d60</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_sharesflag</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a135c13470f475c2094e6cb1e03c1f87a</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_player_card_values</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>aedd00ee3a421afe12a3fafb9c2bca8b6</anchor>
      <arglist>[CARDS777_MAXPLAYERS][hand_size]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_number_cards_drawn</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a98df73dcf4c0700e5f9953e6abf0aeba</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_player_cards</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a70f8b98912f3ec905230bb8fd4309f63</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_no_of_player_cards</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a7bbc5875989acbad854ce8792d7c1e07</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>all_playershares</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a17dcd5f3ed168c574c56b6f9db2076be</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis_d</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a68a976bb37813f4fa860943eecc2779a</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis_b</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a42ef3c7ba5e18cd42816bcb7def71fbf</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>deckid</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>acab83b0ae8f9771b03b3a9a1d51eeffe</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>sharenrs</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>ad0579cac3407f0f0cb742891f36d6e56</anchor>
      <arglist>[256]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>playershares</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a4f81cce59752f55ccee614846f8dfec8</anchor>
      <arglist>[CARDS777_MAXCARDS][CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>struct lws *</type>
      <name>wsi_global_client</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>af76d3fd39038b5f384027b8ecf35298e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct cashier *</type>
      <name>cashier_info</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a28b711b1a0ece6b40a95e022e13c7425</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>max_players</name>
      <anchorfile>dc/d54/common_8h.html</anchorfile>
      <anchor>a097028f2c20ab139a38e5c621193f223</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>gfshare.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>d9/d0a/gfshare_8c</filename>
    <includes id="d4/d71/gfshare_8h" name="gfshare.h" local="yes" imported="no">gfshare.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>void</type>
      <name>libgfshare_init</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a8da60e5c8a3ff68bdf483dc027e326c9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>_gfshare_init_core</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a39ad7d1399a2a4361abbc72c309f4f65</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>gfshare_initenc</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>af4b2071d9c94b528018ec573e39cca05</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>gfshare_initdec</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a53ce4ea8b2a11f4376c43589d11f8113</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>gfshare_sg777_initdec</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a5703f5987bc8c19f45238b53a1a3a005</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_free</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>aae637bf4bae06c3081219ca6d15c0e8d</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_enc_setsecret</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a1b6e4fabe8dd5d7f686962c7e040b2cf</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx, uint8_t *secret)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_encgetshare</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a41d18c5f16254b5161ead9ce861fbc13</anchor>
      <arglist>(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_dec_newshares</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a323be5de03be37cfe4939186746fc988</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx, uint8_t *sharenrs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_dec_giveshare</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a8665725a133ec094f35f51620c6593f0</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_decextract</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a5a30aaf2121bc77e19b85a8f3cb2f6ae</anchor>
      <arglist>(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t *secretbuf)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>gfshare_calc_sharenrs</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a7137856a8a64316ff96b8e6dae47c5a9</anchor>
      <arglist>(uint8_t *sharenrs, int32_t N, uint8_t *data, int32_t datasize)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>gfshare_init_sharenrs</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a1c3c2965cc907b20b57b20cc9ca6f213</anchor>
      <arglist>(uint8_t sharenrs[255], uint8_t *orig, int32_t m, int32_t n)</arglist>
    </member>
    <member kind="function">
      <type>uint8_t *</type>
      <name>gfshare_recoverdata</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a1e3750e6cfdaad8a6495d54d31fc96cb</anchor>
      <arglist>(uint8_t *shares[], uint8_t *sharenrs, int32_t M, uint8_t *recover, int32_t datasize, int32_t N)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_calc_share</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a8e7b5db33b16aed8e9e52f25d30c5284</anchor>
      <arglist>(uint8_t *buffer, int32_t size, int32_t M, uint32_t ilog, uint8_t *share)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_calc_shares</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a9b3942a13178129e7e2188454fdec070</anchor>
      <arglist>(uint8_t *shares, uint8_t *secret, int32_t size, int32_t width, int32_t M, int32_t N, uint8_t *sharenrs, uint8_t *space, int32_t spacesize)</arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>BET_logs</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>a1eb908e2dc8ddd265bd17eb32c2695a6</anchor>
      <arglist>[256]</arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>BET_exps</name>
      <anchorfile>d9/d0a/gfshare_8c.html</anchorfile>
      <anchor>ab449d035ecfc1c7bf4e6e16a6f12e463</anchor>
      <arglist>[510]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>gfshare.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d4/d71/gfshare_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>void</type>
      <name>libgfshare_init</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a8da60e5c8a3ff68bdf483dc027e326c9</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>_gfshare_init_core</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a39ad7d1399a2a4361abbc72c309f4f65</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>gfshare_initenc</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>af4b2071d9c94b528018ec573e39cca05</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>gfshare_initdec</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a53ce4ea8b2a11f4376c43589d11f8113</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>struct gfshare_ctx_bet *</type>
      <name>gfshare_sg777_initdec</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a5703f5987bc8c19f45238b53a1a3a005</anchor>
      <arglist>(uint8_t *sharenrs, uint32_t sharecount, uint8_t threshold, uint32_t size, void *space, int32_t spacesize)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_free</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>aae637bf4bae06c3081219ca6d15c0e8d</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_enc_setsecret</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a1b6e4fabe8dd5d7f686962c7e040b2cf</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx, uint8_t *secret)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_encgetshare</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a41d18c5f16254b5161ead9ce861fbc13</anchor>
      <arglist>(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_dec_newshares</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a323be5de03be37cfe4939186746fc988</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx, uint8_t *sharenrs)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_dec_giveshare</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a8665725a133ec094f35f51620c6593f0</anchor>
      <arglist>(struct gfshare_ctx_bet *ctx, uint8_t sharenr, uint8_t *share)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_decextract</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a5a30aaf2121bc77e19b85a8f3cb2f6ae</anchor>
      <arglist>(uint8_t *_logs, uint8_t *_exps, struct gfshare_ctx_bet *ctx, uint8_t *secretbuf)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>gfshare_calc_sharenrs</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a7137856a8a64316ff96b8e6dae47c5a9</anchor>
      <arglist>(uint8_t *sharenrs, int32_t N, uint8_t *data, int32_t datasize)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>gfshare_init_sharenrs</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a1c3c2965cc907b20b57b20cc9ca6f213</anchor>
      <arglist>(uint8_t sharenrs[255], uint8_t *orig, int32_t m, int32_t n)</arglist>
    </member>
    <member kind="function">
      <type>uint8_t *</type>
      <name>gfshare_recoverdata</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a1e3750e6cfdaad8a6495d54d31fc96cb</anchor>
      <arglist>(uint8_t *shares[], uint8_t *sharenrs, int32_t M, uint8_t *recover, int32_t datasize, int32_t N)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_calc_share</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a8e7b5db33b16aed8e9e52f25d30c5284</anchor>
      <arglist>(uint8_t *buffer, int32_t size, int32_t M, uint32_t ilog, uint8_t *share)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>gfshare_calc_shares</name>
      <anchorfile>d4/d71/gfshare_8h.html</anchorfile>
      <anchor>a9b3942a13178129e7e2188454fdec070</anchor>
      <arglist>(uint8_t *shares, uint8_t *secret, int32_t size, int32_t width, int32_t M, int32_t N, uint8_t *sharenrs, uint8_t *space, int32_t spacesize)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>host.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>df/da9/host_8c</filename>
    <includes id="df/ded/host_8h" name="host.h" local="yes" imported="no">host.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d0/db9/cards777_8h" name="cards777.h" local="yes" imported="no">cards777.h</includes>
    <includes id="d8/de1/client_8h" name="client.h" local="yes" imported="no">client.h</includes>
    <includes id="d5/d90/commands_8h" name="commands.h" local="yes" imported="no">commands.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <includes id="d1/d48/oracle_8h" name="oracle.h" local="yes" imported="no">oracle.h</includes>
    <includes id="d1/d8d/payment_8h" name="payment.h" local="yes" imported="no">payment.h</includes>
    <includes id="d1/dc9/poker_8h" name="poker.h" local="yes" imported="no">poker.h</includes>
    <includes id="db/ddb/states_8h" name="states.h" local="yes" imported="no">states.h</includes>
    <includes id="dd/d98/table_8h" name="table.h" local="yes" imported="no">table.h</includes>
    <member kind="define">
      <type>#define</type>
      <name>LWS_PLUGIN_STATIC</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a6ac88fa6a0ff7931eebef9c3e789b53d</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>dcv_lws_write</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a1f8e3d38c48ef3c4115775937046b4d6</anchor>
      <arglist>(cJSON *data)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_chat</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>af23d0c2074c632617629fce0775356d1</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>initialize_seat</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>aeff84950c67efc57eee0d08ea771874d</anchor>
      <arglist>(cJSON *seatInfo, char *name, int32_t seat, int32_t stack, int32_t empty, int32_t playing)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_seats</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a0766fcd2997389e70a025bb3437716ce</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_game</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>adbe3ab77bc01747b60c29a13f7cd8690</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_frontend</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a42d58e2d2d642d5853edb8cd663c2b7f</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>lws_callback_http_dummy</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a107bec281a1ddc6b98e44b556f95e7bf</anchor>
      <arglist>(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>sigint_handler</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a258e3b580e688a0cf46e4258525aeaf1</anchor>
      <arglist>(int sig)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_push_dcv_to_gui</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a106ab8d73d9b1bb5fcffcbaba60f384e</anchor>
      <arglist>(cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_deck_init_info</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a249bb7c069400cdadfbf5111ca893c0b</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_init</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>af4c0e088e047c4a74e8758ef9a9eb6aa</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_dcv_bvv_join</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a4ef5468acf5b7e9079468541d5c72577</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_start</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a4c71e107f8f247c73fcdeab359a58c04</anchor>
      <arglist>(struct privatebet_info *bet, int32_t peerid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_join_req</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a0b9684e66602d8483270b8f214809cc4</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_send_turn_info</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a5aa313eb5aede5ab3c152519390fb42e</anchor>
      <arglist>(struct privatebet_info *bet, int32_t playerid, int32_t cardid, int32_t card_type)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_turn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a9dc3264b9db7de223419c5551868b6c7</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_relay</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a9d17f9c6b6a2d454798bf5ac36b98ab1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_check_bvv_Ready</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a90bcca4d0aeee5db50f13915bf280409</anchor>
      <arglist>(struct privatebet_info *bet)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_create_invoice</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a936dda54fe149da9e2104428419f24e0</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_create_betting_invoice</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a3d990524438302387f253eb15f4da292</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_check_player_ready</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a45f715b215f34d6b44c72a56a65b6e0e</anchor>
      <arglist>(cJSON *playerReady, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_receive_card</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a8f77d2616f3863fe0fb744607ba48a15</anchor>
      <arglist>(cJSON *playerCardInfo, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_reset</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>af784c4059c06107c85677ad2ff41da26</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_force_reset</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a4722546dfbefaa9c5dc84841b0db31ca</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_evaluate_hand</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a464a280df03afc8add329418712e5ee5</anchor>
      <arglist>(cJSON *playerCardInfo, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_ln_check</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a52ad122e76b59ba9de77f8f8043503e7</anchor>
      <arglist>(struct privatebet_info *bet)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_award_winner</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a195c93135325361eacfada5593143502</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>bet_push_joinInfo</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a47f853e44ecdc0e3406571a7b370fbbb</anchor>
      <arglist>(cJSON *argjson, int32_t numplayers)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_backend</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a853592c099b596451ecd23e99244e1fe</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_backend_loop</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a4a99b05847806e42fbcfc3b5cedc4380</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_frontend_loop</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a2ac04c6502a7b7e3c50f6aaf6cc4eb41</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>bet_send_status</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a68d581d035d443e0076a177bee2f4985</anchor>
      <arglist>(struct privatebet_info *bet)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_live_loop</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>af5ffa99b814c76f0ad597b5a700e6f19</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="variable">
      <type>struct lws *</type>
      <name>wsi_global_host</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>abb636672d53a20859277b4ee50d048f2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>cJSON *</type>
      <name>dcvDataToWrite</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ac05c3f04dded9e82101c72a9f50b382c</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>dcvDataExists</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a2fbd79df5e5b5ce8fe30354a12438913</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>players_joined</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a8b84dc319ea3072597938e05f824fba3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>turn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a0da6d4ab8606b9ebb2ff16fffb343982</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_cards</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ab0b014c77ee4c0f9c4efbbcaaa2233b8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_rounds</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a274d848856a16601b02d0612115d6cdb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_bets</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ac7ede27676d45ec9b8f3300616f3c21d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>card_matrix</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a257fcb6422d25e775ec752cec7e7f00c</anchor>
      <arglist>[CARDS777_MAXPLAYERS][hand_size]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>card_values</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a103f208d345d3203e2111b4ee85b3c9a</anchor>
      <arglist>[CARDS777_MAXPLAYERS][hand_size]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>all_player_cards</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a70f8b98912f3ec905230bb8fd4309f63</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>struct deck_dcv_info</type>
      <name>dcv_info</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ad4f60484e62e04a86ab82adf508155ac</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_ready</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a45143a6ba36953c506de70d9aeb5bf13</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>hole_cards_drawn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>afc92fa9ddca98e6462e2f1fa0b45ce54</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>community_cards_drawn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a9291cd79c30f9d5e6158e9f0e17a4ce3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>flop_cards_drawn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ab778f0489a05381dafaf84814ff42820</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>turn_card_drawn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a8d7ceed4310192c3e202c3d66ac1c2ad</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>river_card_drawn</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a20292555b517e663098d4a6c7ce4301d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>bet_amount</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>afcbc8318d0b36b5385337f1e74843988</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>eval_game_p</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a7518f53581392113f02a0c67ad16b89e</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>eval_game_c</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ad9ccd5e2c002185825b37a9c4ee13264</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_status</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ac8985cbccf62b12d1acb3cd44380bab9</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>bvv_status</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>aa0ac1a7de9a45e00c34202cd8a4c75a8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>player_chips_address</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>adbcd807d45eb5eab19a770d733173623</anchor>
      <arglist>[CARDS777_MAXPLAYERS][64]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>invoiceID</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a391499f348496ec39622c5d6f9e732fd</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>suit</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a9c14867c1a22e34535491feeec626898</anchor>
      <arglist>[NSUITS]</arglist>
    </member>
    <member kind="variable">
      <type>char *</type>
      <name>face</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ad9307743818dc635cc8c74dc1a2a3323</anchor>
      <arglist>[NFACES]</arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_info *</type>
      <name>bet_dcv</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a62b702b575af739ee2c999554ea33c5a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_vars *</type>
      <name>dcv_vars</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ab98cf25f28f49f78a9431fcc11bda58e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>dealerPosition</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ae56101b5edfa437906539229fd5e2d31</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_signers</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a79d6b450d204447564e6deee2bc97d82</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>max_no_of_signers</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>afc2fd24df5d82cbedddde3b28245f46a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>is_signed</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>ae56653403779513733fee433ef0b6aab</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>lws_buf</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a2ce4dbf0f3e8f57ab5d5c40971830609</anchor>
      <arglist>[65536]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>lws_buf_length</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a52aca01cdb7a7e61da5148a62052927d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>pthread_mutex_t</type>
      <name>mutex</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a4acff8232e4aec9cd5c6dc200ac55ef3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static struct lws_protocols</type>
      <name>protocols</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a0cde2a0bf68fbf485a05f22364d5ac5b</anchor>
      <arglist>[]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static int</type>
      <name>interrupted</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>a91a694716d07ab82e16db9cdba7d69c3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static const struct lws_http_mount</type>
      <name>mount</name>
      <anchorfile>df/da9/host_8c.html</anchorfile>
      <anchor>afcd99fd3f865a01dd3dad789fa43e247</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>host.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>df/ded/host_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_deck_init_info</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a249bb7c069400cdadfbf5111ca893c0b</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_init</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>af4c0e088e047c4a74e8758ef9a9eb6aa</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_start</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a4c71e107f8f247c73fcdeab359a58c04</anchor>
      <arglist>(struct privatebet_info *bet, int32_t peerid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_join_req</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a0b9684e66602d8483270b8f214809cc4</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_turn</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a9dc3264b9db7de223419c5551868b6c7</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_relay</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a9d17f9c6b6a2d454798bf5ac36b98ab1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_check_player_ready</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a45f715b215f34d6b44c72a56a65b6e0e</anchor>
      <arglist>(cJSON *playerReady, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_backend</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a853592c099b596451ecd23e99244e1fe</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_backend_loop</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a4a99b05847806e42fbcfc3b5cedc4380</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_receive_card</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a8f77d2616f3863fe0fb744607ba48a15</anchor>
      <arglist>(cJSON *playerCardInfo, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_reset</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>af784c4059c06107c85677ad2ff41da26</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_force_reset</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a4722546dfbefaa9c5dc84841b0db31ca</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_frontend_loop</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a2ac04c6502a7b7e3c50f6aaf6cc4eb41</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_live_loop</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>af5ffa99b814c76f0ad597b5a700e6f19</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_chat</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>af23d0c2074c632617629fce0775356d1</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_seats</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a0766fcd2997389e70a025bb3437716ce</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_game</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>adbe3ab77bc01747b60c29a13f7cd8690</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_frontend</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a42d58e2d2d642d5853edb8cd663c2b7f</anchor>
      <arglist>(struct lws *wsi, cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_evaluate_hand</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a464a280df03afc8add329418712e5ee5</anchor>
      <arglist>(cJSON *playerCardInfo, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_push_dcv_to_gui</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a106ab8d73d9b1bb5fcffcbaba60f384e</anchor>
      <arglist>(cJSON *argjson)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>dcv_lws_write</name>
      <anchorfile>df/ded/host_8h.html</anchorfile>
      <anchor>a1f8e3d38c48ef3c4115775937046b4d6</anchor>
      <arglist>(cJSON *data)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>network.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>d6/d6e/network_8c</filename>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d0/db9/cards777_8h" name="cards777.h" local="yes" imported="no">cards777.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d4/d71/gfshare_8h" name="gfshare.h" local="yes" imported="no">gfshare.h</includes>
    <member kind="function">
      <type>char *</type>
      <name>bet_tcp_sock_address</name>
      <anchorfile>d6/d6e/network_8c.html</anchorfile>
      <anchor>af4d4ffa4bef447a5b2807ebbeecae09b</anchor>
      <arglist>(int32_t bindflag, char *str, char *ipaddr, uint16_t port)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_nanosock</name>
      <anchorfile>d6/d6e/network_8c.html</anchorfile>
      <anchor>a50664af8ed090a6916f103dfa3d62200</anchor>
      <arglist>(int32_t bindflag, char *endpoint, int32_t nntype)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>network.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d9/d94/network_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>char *</type>
      <name>bet_tcp_sock_address</name>
      <anchorfile>d9/d94/network_8h.html</anchorfile>
      <anchor>af4d4ffa4bef447a5b2807ebbeecae09b</anchor>
      <arglist>(int32_t bindflag, char *str, char *ipaddr, uint16_t port)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_nanosock</name>
      <anchorfile>d9/d94/network_8h.html</anchorfile>
      <anchor>a50664af8ed090a6916f103dfa3d62200</anchor>
      <arglist>(int32_t bindflag, char *endpoint, int32_t nntype)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>oracle.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>d1/d52/oracle_8c</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d0/db9/cards777_8h" name="cards777.h" local="yes" imported="no">cards777.h</includes>
  </compound>
  <compound kind="file">
    <name>oracle.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d1/d48/oracle_8h</filename>
  </compound>
  <compound kind="file">
    <name>payment.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>d5/df6/payment_8c</filename>
    <includes id="d1/d8d/payment_8h" name="payment.h" local="yes" imported="no">payment.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d8/de1/client_8h" name="client.h" local="yes" imported="no">client.h</includes>
    <includes id="d5/d90/commands_8h" name="commands.h" local="yes" imported="no">commands.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_create_invoice_request</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>ac6d0205dc12619aabd56bb33b19c5dca</anchor>
      <arglist>(struct privatebet_info *bet, int32_t playerid, int32_t amount)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_invoice_pay</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>a6a9f6e88a2282777d4bb16d4543b0f21</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars, int playerid, int amount)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_pay</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>a7376b85fc15c7be9f536476a74e77448</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_paymentloop</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>a4e708bf49a4334447149932fd3769b8e</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_create_invoice</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>ab0650f7f42f07c8226e2ca181ff06a1b</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, char *deckid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_create_invoice_request</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>a33f41d1c8988f75b6176fbf77341c857</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, int32_t amount)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_invoice_request</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>a1ca495dfbbb1646f98487c2e2604a198</anchor>
      <arglist>(cJSON *argjson, cJSON *actionResponse, struct privatebet_info *bet, int32_t amount)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_invoice_pay</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>ad94f72c0f7b2bd5e29709db97cf0c905</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int amount)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_player_paymentloop</name>
      <anchorfile>d5/df6/payment_8c.html</anchorfile>
      <anchor>ac266d8f63e96ef42fa89ebb5d2c90500</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>payment.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d1/d8d/payment_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_pay</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>a7376b85fc15c7be9f536476a74e77448</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_create_invoice_request</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>ac6d0205dc12619aabd56bb33b19c5dca</anchor>
      <arglist>(struct privatebet_info *bet, int32_t playerid, int32_t amount)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_invoice_pay</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>a6a9f6e88a2282777d4bb16d4543b0f21</anchor>
      <arglist>(struct privatebet_info *bet, struct privatebet_vars *vars, int playerid, int amount)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_dcv_paymentloop</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>a4e708bf49a4334447149932fd3769b8e</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_create_invoice</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>ab0650f7f42f07c8226e2ca181ff06a1b</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, char *deckid)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_create_invoice_request</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>a33f41d1c8988f75b6176fbf77341c857</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, int32_t amount)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_invoice_pay</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>ad94f72c0f7b2bd5e29709db97cf0c905</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars, int amount)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>bet_player_paymentloop</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>ac266d8f63e96ef42fa89ebb5d2c90500</anchor>
      <arglist>(void *_ptr)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_invoice_request</name>
      <anchorfile>d1/d8d/payment_8h.html</anchorfile>
      <anchor>a1ca495dfbbb1646f98487c2e2604a198</anchor>
      <arglist>(cJSON *argjson, cJSON *actionResponse, struct privatebet_info *bet, int32_t amount)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>poker.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>db/dd5/poker_8c</filename>
    <class kind="struct">CardPileType</class>
    <member kind="define">
      <type>#define</type>
      <name>CLUB_SUIT</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a613059795622f7f32edd2d3b6ddd3660</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>DIAMOND_SUIT</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>ac6da7b0d6a04884878334f1529992d8e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>HEART_SUIT</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a2783635b64e96f42c27bdd1a5c649321</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SPADE_SUIT</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a0a474986223e8ad48c42d2b8d8c654a3</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>RANK_SHL</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>abe32f004631c4ff9b9f878e83698de7e</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SUBR_SHL</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a43695e1778b5715e372fc5b1bf3eef04</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SUBR_SHLMASK</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>abf7eea484f5824b302f8231bedead0b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>STRAIGHT_FLUSH_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>aca8ab57a5490e797ec73ce6c850f8f5b</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FOUR_KIND_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>af092d1170e8174c285ec3d7ac9c352a8</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FULL_HOUSE_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a32f7788b2d25205f136370e3efcee980</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>FLUSH_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a54917d5c33d9e8efa83b40ee7004baee</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>STRAIGHT_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a76b585f6b88d59f40aa54f9234d56b8a</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>THREE_KIND_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>aaa4fd936a91a0d9bc7b8a480786892ed</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TWO_PAIR_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>ac6532f9413c1cbf003813be93e0f3a24</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>TWO_KIND_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a7df5076cc3497a919f8dc71f154971ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>ONE_PAIR_SCORE</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a7e12e423c0afaf2ac5a0d714204b8243</anchor>
      <arglist></arglist>
    </member>
    <member kind="define">
      <type>#define</type>
      <name>SevenCardDrawFlushScore</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>aac1db89700e6424f3d153b6b3b0d7fb6</anchor>
      <arglist>(f)</arglist>
    </member>
    <member kind="typedef">
      <type>uint8_t</type>
      <name>u8</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a92c50087ca0e64fa93fc59402c55f8ca</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>int8_t</type>
      <name>s8</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a9e382f207c65ca13ab4ae98363aeda80</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>uint32_t</type>
      <name>u32</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>afaa62991928fb9fb18ff0db62a040aba</anchor>
      <arglist></arglist>
    </member>
    <member kind="typedef">
      <type>int32_t</type>
      <name>s32</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>ae9b1af5c037e57a98884758875d3a7c4</anchor>
      <arglist></arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>cardstr</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a9b6a6c7ff344c8a7823c0e67fb072772</anchor>
      <arglist>(char *cardstr, uint8_t card)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static u32</type>
      <name>FiveCardDrawScoreFast</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>afe9491d9d9532c0ab6cc581642931a5d</anchor>
      <arglist>(u32 c0, u32 c1, u32 c2, u32 c3, u32 c4, u32 u)</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>five_card_draw_score</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>af6a626651997d57da7ff0f9184b991a0</anchor>
      <arglist>(const u8 *h)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static u32</type>
      <name>SevenCardDrawFlush</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>adbf73cb45bbbf4297b03e18bbfd533c5</anchor>
      <arglist>(const u8 *h, const u32 c[7])</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>seven_card_draw_score</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a7105eec27c81da5abbc505946b417bf1</anchor>
      <arglist>(const u8 *h)</arglist>
    </member>
    <member kind="function">
      <type>u32</type>
      <name>SevenCardDrawScoreSlow</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a6ab75915cc9017e3217b5cbb80bba5b3</anchor>
      <arglist>(const u8 *h)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>Shuffle</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a40cffff9fbc138de76f603a6f7e8c9ca</anchor>
      <arglist>(CardPileType *c)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static int32_t</type>
      <name>Deal</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a4f63fa5aeea5b926806694d772a66fc6</anchor>
      <arglist>(CardPileType *h, CardPileType *d, int32_t n)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>InitDeck</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a47b0e41137b46259ca5e050fdfaa9bab</anchor>
      <arglist>(CardPileType *deck)</arglist>
    </member>
    <member kind="function" static="yes">
      <type>static void</type>
      <name>DisplayCard</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a0507ecc305af4038e505f2000406aa85</anchor>
      <arglist>(u8 c, char out[])</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>DisplayHand5</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>acef1a3deae5b92dc182a742281a81bd8</anchor>
      <arglist>(const CardPileType *h)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set_cardstr</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a9db86ea390a878443149b0c92968ed37</anchor>
      <arglist>(char *cardstr, uint32_t c)</arglist>
    </member>
    <member kind="function">
      <type>uint32_t</type>
      <name>set_handstr</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>acdc182ed757e42020f170f27b5abe278</anchor>
      <arglist>(char *handstr, uint8_t cards[7], int32_t verbose)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>DisplayHand7</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>abba617f9153e929f0305576e8c761993</anchor>
      <arglist>(char *handstr, uint8_t *cards)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>poker_test</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a0ded1fa0dbc3debd1a36333887723a20</anchor>
      <arglist>()</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static char *</type>
      <name>handstrs</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a1febeba6ae7e1e4aa4bdcacf444e7baf</anchor>
      <arglist>[16]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static char *</type>
      <name>kickerstrs</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>aaab18c836f7ea862caec703454865c7b</anchor>
      <arglist>[16]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static u32</type>
      <name>CardValue</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a2e3b6dec6fd2052e145b496a12885039</anchor>
      <arglist>[52]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static u32</type>
      <name>CardSuit</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>a1b7df48baac6409350528ce8ac4399f5</anchor>
      <arglist>[52]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static u32</type>
      <name>CardSuitIdx</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>aba5a822926da9c10d159f0c1657b4e5b</anchor>
      <arglist>[52]</arglist>
    </member>
    <member kind="variable" static="yes">
      <type>static u32</type>
      <name>CardMask</name>
      <anchorfile>db/dd5/poker_8c.html</anchorfile>
      <anchor>ade63573b26953751edff9c57b993baf7</anchor>
      <arglist>[52]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>poker.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>d1/dc9/poker_8h</filename>
    <member kind="function">
      <type>unsigned long</type>
      <name>five_card_draw_score</name>
      <anchorfile>d1/dc9/poker_8h.html</anchorfile>
      <anchor>aed5a2d96cb8e977e8dc5e9e79a572957</anchor>
      <arglist>(const unsigned char *h)</arglist>
    </member>
    <member kind="function">
      <type>unsigned long</type>
      <name>seven_card_draw_score</name>
      <anchorfile>d1/dc9/poker_8h.html</anchorfile>
      <anchor>af22de0b0993873cf3de0e90d4647dc9d</anchor>
      <arglist>(const unsigned char *h)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>states.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>dc/d3d/states_8c</filename>
    <includes id="db/ddb/states_8h" name="states.h" local="yes" imported="no">states.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="d8/de1/client_8h" name="client.h" local="yes" imported="no">client.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="df/ded/host_8h" name="host.h" local="yes" imported="no">host.h</includes>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <includes id="d1/d8d/payment_8h" name="payment.h" local="yes" imported="no">payment.h</includes>
    <includes id="dd/d98/table_8h" name="table.h" local="yes" imported="no">table.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_initiate_statemachine</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>aa5e8195151ba93d916d0205d6b4e1a5a</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_next_turn</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a2a03cc87f22e23a1c80dbd6b4381ab44</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_round_betting</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a84d0ac153cb6d18766ef9cf028e2ab46</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_round_betting_response</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a3a113671749a2199137e2a5e6c6483e0</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_big_blind_bet</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a350e46d32b1aa7449246d7073ad18c0f</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_big_blind</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>aa35e9b29d2808ee409dda4356d27c60e</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_small_blind_bet</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a61cf370e1a83699635bee7f1abdf9898</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_small_blind</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a49d11ebe4cfbf21c7de795c45252d50c</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_betting_statemachine</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>af5777fe57cd0b5a40eb856adbdc83cd1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_display_current_state</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a7a3dc4faee054241317868b6b001a583</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_small_blind_bet</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a7c0dce7f16194239b75ff85f61a4c026</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_big_blind_bet</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a47453f02b2dac55db8ed937ab704e9f8</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_dealer_info</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a8abe247566a58f5ad26e359cd97ed24c</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_small_blind</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a4217e97cd49f3bc8f1678c4194680a4c</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_big_blind</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a40fe07492c4c41bf376a7350ed014b68</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_round_betting</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a82afe6df9ab05a1437f8b373b00313a4</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_round_betting_response</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>ab602935874a768d0852d3b1e666b9ed1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>action_str</name>
      <anchorfile>dc/d3d/states_8c.html</anchorfile>
      <anchor>a4b5d8caf832a6b74a80d5cefd1b012f1</anchor>
      <arglist>[8][100]</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>states.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>db/ddb/states_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_initiate_statemachine</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>aa5e8195151ba93d916d0205d6b4e1a5a</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_big_blind_bet</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a350e46d32b1aa7449246d7073ad18c0f</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_big_blind</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>aa35e9b29d2808ee409dda4356d27c60e</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_small_blind_bet</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a61cf370e1a83699635bee7f1abdf9898</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_round_betting</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a84d0ac153cb6d18766ef9cf028e2ab46</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_round_betting_response</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a3a113671749a2199137e2a5e6c6483e0</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_next_turn</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a2a03cc87f22e23a1c80dbd6b4381ab44</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_big_blind</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a40fe07492c4c41bf376a7350ed014b68</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_dcv_small_blind</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a49d11ebe4cfbf21c7de795c45252d50c</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_betting_statemachine</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>af5777fe57cd0b5a40eb856adbdc83cd1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_display_current_state</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a7a3dc4faee054241317868b6b001a583</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_small_blind_bet</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a7c0dce7f16194239b75ff85f61a4c026</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_big_blind_bet</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a47453f02b2dac55db8ed937ab704e9f8</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_dealer_info</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a8abe247566a58f5ad26e359cd97ed24c</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_small_blind</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a4217e97cd49f3bc8f1678c4194680a4c</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_round_betting_response</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>ab602935874a768d0852d3b1e666b9ed1</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
    <member kind="function">
      <type>int32_t</type>
      <name>bet_player_round_betting</name>
      <anchorfile>db/ddb/states_8h.html</anchorfile>
      <anchor>a82afe6df9ab05a1437f8b373b00313a4</anchor>
      <arglist>(cJSON *argjson, struct privatebet_info *bet, struct privatebet_vars *vars)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>table.c</name>
    <path>/root/bet/privatebet/</path>
    <filename>dc/d33/table_8c</filename>
    <includes id="dd/d98/table_8h" name="table.h" local="yes" imported="no">table.h</includes>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <includes id="dc/d54/common_8h" name="common.h" local="yes" imported="no">common.h</includes>
    <includes id="d9/d94/network_8h" name="network.h" local="yes" imported="no">network.h</includes>
    <member kind="function">
      <type>void</type>
      <name>bet_info_set</name>
      <anchorfile>dc/d33/table_8c.html</anchorfile>
      <anchor>abd3fa7b74815028b49eecac87d9efe33</anchor>
      <arglist>(struct privatebet_info *bet, char *game, int32_t range, int32_t numrounds, int32_t maxplayers)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>table.h</name>
    <path>/root/bet/privatebet/</path>
    <filename>dd/d98/table_8h</filename>
    <includes id="df/d5a/bet_8h" name="bet.h" local="yes" imported="no">bet.h</includes>
    <member kind="function">
      <type>void</type>
      <name>bet_info_set</name>
      <anchorfile>dd/d98/table_8h.html</anchorfile>
      <anchor>abd3fa7b74815028b49eecac87d9efe33</anchor>
      <arglist>(struct privatebet_info *bet, char *game, int32_t range, int32_t numrounds, int32_t maxplayers)</arglist>
    </member>
  </compound>
  <compound kind="file">
    <name>README.md</name>
    <path>/root/bet/</path>
    <filename>d9/dd6/_r_e_a_d_m_e_8md</filename>
  </compound>
  <compound kind="struct">
    <name>BET_shardsinfo</name>
    <filename>d4/dce/struct_b_e_t__shardsinfo.html</filename>
    <member kind="variable">
      <type>UT_hash_handle</type>
      <name>hh</name>
      <anchorfile>d4/dce/struct_b_e_t__shardsinfo.html</anchorfile>
      <anchor>a6553a2f82e2eaa698bc743d7214cacca</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>numcards</name>
      <anchorfile>d4/dce/struct_b_e_t__shardsinfo.html</anchorfile>
      <anchor>a51fbef7e339f923d13381b8e302defe2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>numplayers</name>
      <anchorfile>d4/dce/struct_b_e_t__shardsinfo.html</anchorfile>
      <anchor>a0873504d0f2c17a1a4e6983e62903857</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>key</name>
      <anchorfile>d4/dce/struct_b_e_t__shardsinfo.html</anchorfile>
      <anchor>aa90bedeb796a7940ff452191c5405a31</anchor>
      <arglist>[sizeof(bits256)+2 *sizeof(int32_t)]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>recover</name>
      <anchorfile>d4/dce/struct_b_e_t__shardsinfo.html</anchorfile>
      <anchor>a293358259dce8c51fbdeaf1b6323e495</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>data</name>
      <anchorfile>d4/dce/struct_b_e_t__shardsinfo.html</anchorfile>
      <anchor>ac11f71d5b02d8113ab4c2764505deca0</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>CardPileType</name>
    <filename>dd/dfc/struct_card_pile_type.html</filename>
    <member kind="variable">
      <type>int32_t</type>
      <name>len</name>
      <anchorfile>dd/dfc/struct_card_pile_type.html</anchorfile>
      <anchor>a1eb13d658b445dd4fd8a27dd039d098a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>u8</type>
      <name>entry</name>
      <anchorfile>dd/dfc/struct_card_pile_type.html</anchorfile>
      <anchor>a10602423899018c878601a1a8d833257</anchor>
      <arglist>[52]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>cashier</name>
    <filename>da/d77/structcashier.html</filename>
    <member kind="variable">
      <type>int32_t</type>
      <name>pullsock</name>
      <anchorfile>da/d77/structcashier.html</anchorfile>
      <anchor>a0b5bb840a70a75c97b3a2ea956e38d74</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pubsock</name>
      <anchorfile>da/d77/structcashier.html</anchorfile>
      <anchor>a446347ed86dea33ee1b83be7b78fe4c9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>addr</name>
      <anchorfile>da/d77/structcashier.html</anchorfile>
      <anchor>ac1cabcc8e9aad13e98becac8c3effccb</anchor>
      <arglist>[67]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>deck_bvv_info</name>
    <filename>d9/d0c/structdeck__bvv__info.html</filename>
    <member kind="variable">
      <type>bits256</type>
      <name>deckid</name>
      <anchorfile>d9/d0c/structdeck__bvv__info.html</anchorfile>
      <anchor>a33dcbc6d6f96dd7270ddd0dc2f418b25</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis</name>
      <anchorfile>d9/d0c/structdeck__bvv__info.html</anchorfile>
      <anchor>ab8b8de70d12f184c0ac81af66ca5c083</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>struct pair256</type>
      <name>bvv_key</name>
      <anchorfile>d9/d0c/structdeck__bvv__info.html</anchorfile>
      <anchor>a7b011d6e18b84f26dd65d24b631d6d24</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>bvvblindcards</name>
      <anchorfile>d9/d0c/structdeck__bvv__info.html</anchorfile>
      <anchor>a1555643bed8aed064081d11c3895807a</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>numplayers</name>
      <anchorfile>d9/d0c/structdeck__bvv__info.html</anchorfile>
      <anchor>a498eddcaa0edbf7e9993c97e34bfdb81</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>maxplayers</name>
      <anchorfile>d9/d0c/structdeck__bvv__info.html</anchorfile>
      <anchor>a6afb9fe25b3093496c0151d564b17175</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>deck_dcv_info</name>
    <filename>d5/dc7/structdeck__dcv__info.html</filename>
    <member kind="variable">
      <type>bits256</type>
      <name>deckid</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>afcae07e0ab2f88ef3e27e5d3c2e8dd51</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct pair256</type>
      <name>dcv_key</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>af771836c5b878645f37a534b4bdc4daa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a25fd0412547703e73a77605912374b38</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>cardpubkeys</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>ae031536026c1fa0fa5c9d2ee431f512d</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>dcvblindcards</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a96eea74bbc4a46093273b82ac13dfcef</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>cardprods</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a25b24572c534cb608259e8f767d95469</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>peerpubkeys</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>ab353e16283749de7b5ce105f864733e3</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>numplayers</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a533796e39e541718a2aab0d61414612b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>maxplayers</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a95db599c4f330024040c6c9b7c46d5ef</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>uri</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>ac019b3b109ead05c7d6d76846603399b</anchor>
      <arglist>[CARDS777_MAXPLAYERS][100]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>betamount</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>abf784ce3b599f9ca61608210559c56f9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>commision</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a49c38791b251048ff83d948f76154441</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>paidamount</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a7cf6f1023e1245a8b19b87f63fa64e7a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>unsigned char</type>
      <name>bvv_uri</name>
      <anchorfile>d5/dc7/structdeck__dcv__info.html</anchorfile>
      <anchor>a1415902499c43e3c0a4ba1f75fbdfeaf</anchor>
      <arglist>[100]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>deck_player_info</name>
    <filename>d0/d09/structdeck__player__info.html</filename>
    <member kind="variable">
      <type>struct pair256</type>
      <name>player_key</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a69c860bb618326b42649fb8dc874b19a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>cardpubkeys</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a42b58a3d6acbef59ef0951a2b441c1cd</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>cardprivkeys</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>aace068630cb6fbc45a944df6d533e91e</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a3627ccd09a0aeedd9e72dcd69186181b</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>cardprods</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>afe50d0d657541e924497e3e16765abf9</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>bvvblindcards</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a8c853de455fc2b5289789287a025cb05</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>dcvpubkey</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>addcc08f5394486871a4fbec618c84df9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>bvvpubkey</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>ada2129496132e81e7a4ca7ec2586226e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>deckid</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>aa88dd303072d38aa72f735ea385fbbc6</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>numplayers</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a07a5c22a6eb96173dd6cbd0262ad6968</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>maxplayers</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a28099f450784fc67babc668315a61318</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>numcards</name>
      <anchorfile>d0/d09/structdeck__player__info.html</anchorfile>
      <anchor>a1f07b1a6921a7a2c1055a47ff6dc8ae0</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>enc_share</name>
    <filename>d0/d0d/structenc__share.html</filename>
    <member kind="variable">
      <type>uint8_t</type>
      <name>bytes</name>
      <anchorfile>d0/d0d/structenc__share.html</anchorfile>
      <anchor>a4bac0ffac6bcc7a207ef17db51b8c471</anchor>
      <arglist>[sizeof(bits256)+crypto_box_NONCEBYTES+crypto_box_ZEROBYTES]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>gfshare_ctx_bet</name>
    <filename>d8/d1f/structgfshare__ctx__bet.html</filename>
    <member kind="variable">
      <type>uint32_t</type>
      <name>sharecount</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>a4c765415b8d842a92c6644b59e036a3f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>threshold</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>ae63c84f60d45927b32f01ee03bc75bb8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>size</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>aadffa692ab06b96d49f1e38024707a03</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>buffersize</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>a6d3cec2d48e3a5530f2fb955085487d0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>allocsize</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>a8ae395c7d798dfc19764f0ccdbe8e405</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>sharenrs</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>a93046b5752b076392fbc010cf97bec2c</anchor>
      <arglist>[255]</arglist>
    </member>
    <member kind="variable">
      <type>uint8_t</type>
      <name>buffer</name>
      <anchorfile>d8/d1f/structgfshare__ctx__bet.html</anchorfile>
      <anchor>a14d97810d00ad0a397b006c3b32eb4b5</anchor>
      <arglist>[]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>pair256</name>
    <filename>de/df9/structpair256.html</filename>
    <member kind="variable">
      <type>bits256</type>
      <name>priv</name>
      <anchorfile>de/df9/structpair256.html</anchorfile>
      <anchor>a08e3b8e0e895b13eebc705b35e6113aa</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>prod</name>
      <anchorfile>de/df9/structpair256.html</anchorfile>
      <anchor>a3d40c38f9e09a72c32f584c783528c7a</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>privatebet_info</name>
    <filename>d0/d9f/structprivatebet__info.html</filename>
    <member kind="variable">
      <type>char</type>
      <name>game</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a9f134d6dc6e24c2382cc9a937a8c4fd9</anchor>
      <arglist>[64]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>MofN</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ac33fc414297eac35e0e50eefcce9e358</anchor>
      <arglist>[CARDS777_MAXCARDS *CARDS777_MAXPLAYERS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>cardpubs</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a8f79ab33689ddb0e10858491ec02a896</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>playerpubs</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ad834b583e3a2f3b8607420c42751e376</anchor>
      <arglist>[CARDS777_MAXPLAYERS+1]</arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>tableid</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a2092f6f498aad90aa677505b82e48c9e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>deckid</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ac742825adc9a2808a8e0d18390e18b24</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>numplayers</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a8a60dda249031fc496a0307c34f3cb69</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>maxplayers</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a2d44b99e9160029507e6e0d04b92dbd4</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>numrounds</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ae9386be7e568fdb22e00cb600834fc7b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>range</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>af01242479bf0e4175782043883b177f8</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>myplayerid</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ad502ebd5a1f56f383db682da311e813e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>maxchips</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a195fee78bb1a26504e4920dafea3b195</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>chipsize</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a1f4c834e20b87839446da22179d2e58b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pullsock</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a5a624f46decb6be461ec1e4edc0cc140</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pubsock</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a2443f8a982fb92f3cab78d35d54aeb24</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>subsock</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>adf0111f2d0328b929100e926427b5ced</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pushsock</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>aa04b8bcf4acf5ced8cfeaa006cc69249</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>timestamp</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a41d485812c76258fe86aa2ebb0a43e3b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>peerids</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a3aa8ce19463aa786e93e92ac4759a6bd</anchor>
      <arglist>[CARDS777_MAXPLAYERS+1][67]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>cardid</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ac1a593b6b04f50e52892def22a3fdd53</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>turni</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>a4a3070b8d22e8ce276a72d45db56353e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>no_of_turns</name>
      <anchorfile>d0/d9f/structprivatebet__info.html</anchorfile>
      <anchor>ad37d01428dc307e9deaf5768596e94ef</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>privatebet_peerln</name>
    <filename>df/d2b/structprivatebet__peerln.html</filename>
    <member kind="variable">
      <type>int32_t</type>
      <name>numrhashes</name>
      <anchorfile>df/d2b/structprivatebet__peerln.html</anchorfile>
      <anchor>accd8b1a6dd4f84596603abfe1401c886</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>numpaid</name>
      <anchorfile>df/d2b/structprivatebet__peerln.html</anchorfile>
      <anchor>abcb19c688a0e4f8c4e4854079526899f</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>hostrhash</name>
      <anchorfile>df/d2b/structprivatebet__peerln.html</anchorfile>
      <anchor>a1bdfd627f7ae81d4b09a245173e42773</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>clientrhash</name>
      <anchorfile>df/d2b/structprivatebet__peerln.html</anchorfile>
      <anchor>a0260b68acba6b1658e2e2b73b2586cdc</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>clientpubkey</name>
      <anchorfile>df/d2b/structprivatebet__peerln.html</anchorfile>
      <anchor>a6324e33f89ff4379976cfa983bc58065</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct privatebet_rawpeerln</type>
      <name>raw</name>
      <anchorfile>df/d2b/structprivatebet__peerln.html</anchorfile>
      <anchor>ab2eaa65aa69210d0dc5431f5d3a27b56</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>privatebet_rawpeerln</name>
    <filename>de/d2d/structprivatebet__rawpeerln.html</filename>
    <member kind="variable">
      <type>uint64_t</type>
      <name>msatoshi_to_us</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>af7884913a1c30729415f2adcc5d24ea3</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint64_t</type>
      <name>msatoshi_total</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>a21761ed0493fefc629ad0c283118edbf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>unique_id</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>aafdd221293a0b3c26dd6648988885c9e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>peerid</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>a12c96ca3817b6f8a222dd5b8420ebee5</anchor>
      <arglist>[67]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>channel</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>ac23c7f3234956f63ad4d0948133784d2</anchor>
      <arglist>[64]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>netaddr</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>a8ded3169d57b93d556f08f667ea8db51</anchor>
      <arglist>[64]</arglist>
    </member>
    <member kind="variable">
      <type>char</type>
      <name>state</name>
      <anchorfile>de/d2d/structprivatebet__rawpeerln.html</anchorfile>
      <anchor>a9125d772164f03871992f971d06624eb</anchor>
      <arglist>[32]</arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>privatebet_share</name>
    <filename>db/db3/structprivatebet__share.html</filename>
    <member kind="variable">
      <type>int32_t</type>
      <name>numplayers</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>a68fd85a4a8e37ea44874f539eae7b351</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>range</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>a3cf44241e4e8ce86848648d66dae414a</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>myplayerid</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>a7ed135d259a8397ff4d14873d2a3eee9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pullsock</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>aae07563369523806f9ac464dba5a0213</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pubsock</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>add6af3c7c7347f299ba30405ebd4ebc0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>subsock</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>a4214acb498a8560837dea48ee40a2746</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pushsock</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>af3be21301f453b7db1c634ac4edfe1ce</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>bvv_public_key</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>ac6419dd53a9675112380945ecc31df2d</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>struct pair256</type>
      <name>player_key</name>
      <anchorfile>db/db3/structprivatebet__share.html</anchorfile>
      <anchor>a890a65950223041a020fe0dd905c6991</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>privatebet_vars</name>
    <filename>d7/d03/structprivatebet__vars.html</filename>
    <member kind="variable">
      <type>bits256</type>
      <name>myhash</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a3d1f69a4d06de80930cd37e647deb1e9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bits256</type>
      <name>hashes</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a66da046d580bae150ab99dd6a5ac3b55</anchor>
      <arglist>[CARDS777_MAXPLAYERS+1][2]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permis</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>af2d141af405be918e44dd233bb2c28ce</anchor>
      <arglist>[CARDS777_MAXPLAYERS+1][CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>endround</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a1024da5f14f9d2a5feea553160943bc9</anchor>
      <arglist>[CARDS777_MAXPLAYERS+1]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>evalcrcs</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a47a408e1bf5e3ac50fb9321a6f00332d</anchor>
      <arglist>[CARDS777_MAXPLAYERS+1]</arglist>
    </member>
    <member kind="variable">
      <type>uint32_t</type>
      <name>consensus</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a4bd5b696098a9133c7d7f6b601c1e061</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>cJSON *</type>
      <name>actions</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a9cc412a623fde59fcd173ca97b504cde</anchor>
      <arglist>[CARDS777_MAXROUNDS][CARDS777_MAXPLAYERS+1]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>mypermi</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>acd3817a80f1de7845e27291c550b0e92</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>permi</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>ab84d5fd502ed68296043e4b42a19398f</anchor>
      <arglist>[CARDS777_MAXCARDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>turni</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a9bda4d6f988f69e54218f62c3bcd1233</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>round</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a184f9ce3af1e6e3b08aa2e6d9b0cdbf1</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>validperms</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a73fcbc896674210a5223a4882ac4155b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>roundready</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>ad73f44b3265d431c0c24ae28c4e13f72</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>lastround</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a790a7f2f8be1750bdf9cb427757e05cb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>numconsensus</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>ae78c3f094c017e1eceb76b38a9191f12</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>small_blind</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a6ceca0aa346ee52a267954a93a0310d5</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>big_blind</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>ae12d2ae74edbf729a736f7ab11479757</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>betamount</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>ad76c9dddaf0a04d2ebb1e9ee95d3c8af</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>bet_actions</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a381939fb9b7f85c0b09f3af728eb5b42</anchor>
      <arglist>[CARDS777_MAXPLAYERS][CARDS777_MAXROUNDS]</arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>dealer</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a9fabccec02611d3882520e04ebf0827e</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>last_turn</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a010311c27ac57ee8379a1c378ccc2dbf</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>last_raise</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>afee2532af1662a5b177f02207be87704</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>pot</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a1b05ab6687bf00d8f08bfd424531e795</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>player_funds</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>a0b69fe1b272370c73a04671e4a04fff0</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int32_t</type>
      <name>funds</name>
      <anchorfile>d7/d03/structprivatebet__vars.html</anchorfile>
      <anchor>acb9cb80d690397299df31c2495f5ae87</anchor>
      <arglist>[CARDS777_MAXPLAYERS]</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>md__root_bet_README</name>
    <title>Pangea-Bet</title>
    <filename>d9/da3/md__root_bet__r_e_a_d_m_e</filename>
  </compound>
</tagfile>
