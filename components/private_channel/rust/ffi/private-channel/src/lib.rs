extern crate curve25519_dalek;
extern crate sha2;

use curve25519_dalek::scalar::Scalar;
use elgamal_ristretto::ciphertext::Ciphertext;
use elgamal_ristretto::private::SecretKey;
use repsys_crypto::{
    combine_pks, encrypt_input, generate_keys, partial_decryption_and_proof, randomize_and_prove,
};

use sha2::Sha512;
use bincode;

pub struct FirstRoundOutput {
    pub pkey: Vec<u8>,
    pub skey: Vec<u8>,
    pub shared_pk: Vec<u8>,
    pub enc_hashes: Vec<u8>,
}

pub struct SecondRoundOutput {
    pub partial_dec: Vec<u8>,    
    pub proofs: Vec<u8>,
    pub rand_vec: Vec<u8>,
}

pub fn start_challenge(
    input: Vec<String>,
    server_pk_encoded: [u8; 32],
) -> Result<FirstRoundOutput, ()> {
    let pk_server = bincode::deserialize(&server_pk_encoded).map_err(|_| ())?;

    let vector_hashes: Vec<Scalar> = input.iter().map(|input_n| {
        Scalar::hash_from_bytes::<Sha512>(input_n.as_bytes())
    }).collect();

    // generate new key pair at every round
    let (sk_client, pk_client) = generate_keys();
    let shared_pk = combine_pks(pk_client, pk_server);

    let encrypted_hashes = encrypt_input(shared_pk, &vector_hashes);

    let encoded_pk = bincode::serialize(&pk_client).map_err(|_| ())?;
    let encoded_sk = bincode::serialize(&sk_client).map_err(|_| ())?;
    let encoded_shared_pk = bincode::serialize(&shared_pk).map_err(|_| ())?;
    let encoded_enc_hashes = bincode::serialize(&encrypted_hashes).map_err(|_| ())?;

    Ok(FirstRoundOutput{
        pkey: encoded_pk,
        skey: encoded_sk,
        shared_pk: encoded_shared_pk,
        enc_hashes: encoded_enc_hashes,
    })
}

pub fn second_round(input: &[u8], encoded_sk: &[u8]) -> Result<SecondRoundOutput, ()> {
    let sk: SecretKey = bincode::deserialize(encoded_sk).map_err(|_| ())?;
    let enc_checks: Vec<Ciphertext> = bincode::deserialize(input).map_err(|_| ())?;

    let (rand_vec, _) = randomize_and_prove(&enc_checks);
    let (partial_decryption, proofs_correct_decryption) =
        partial_decryption_and_proof(&rand_vec, &sk);

    let encoded_partial_decryption = bincode::serialize(&partial_decryption).map_err(|_| ())?;
    let encoded_proofs = bincode::serialize(&proofs_correct_decryption).map_err(|_| ())?;
    let encoded_rand_vec = bincode::serialize(&rand_vec).map_err(|_| ())?;
    
    Ok(SecondRoundOutput {
        partial_dec: encoded_partial_decryption,
        proofs: encoded_proofs,
        rand_vec: encoded_rand_vec,
    })
}