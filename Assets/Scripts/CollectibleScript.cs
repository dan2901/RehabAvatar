using UnityEngine;

public class CollectibleScript : MonoBehaviour
{     
    public float rotationSpeed = 3.0f; //for the cubes to rotate
    public AudioSource collectibleSound;
    public GameObject onCollectEffect;
    public GameObject whichRespawn;
    int i = 0;

    //the following are pre-set locations for the cubes to respawn
    private Vector3[] respawnPositions = new Vector3[] {
        //new Vector3(-0.152999997f,1.579f,-1.755f),
        //new Vector3(0.351000011f,1.52999997f,-0.626999974f),
        new Vector3(-1.94000006f,2.1400001f,-0.626999974f),
        new Vector3(0.351000011f,1.52999997f,-0.626999974f),
        new Vector3(1.57000005f,2.22099996f,-0.626999974f),
        new Vector3(-1.94000006f,2.1400001f,-0.626999974f),
        new Vector3(0.351000011f,1.52999997f,-0.626999974f),
        new Vector3(-1.94000006f,2.1400001f,-0.626999974f),
        new Vector3(1.57000005f,2.22099996f,-0.626999974f),
        new Vector3(-1.94000006f,2.1400001f,-0.626999974f),
        new Vector3(0.351000011f,1.52999997f,-0.626999974f),
        new Vector3(1.57000005f,2.22099996f,-0.626999974f),
    };
    
    void Update()
    {
        transform.Rotate(0,rotationSpeed,0);
    }

    //what happens when the foot touches a cube
    private void OnTriggerEnter(Collider other)
    {
        if (other.tag == "forCollectible")
        {

            Instantiate(onCollectEffect, transform.position, transform.rotation);
            collectibleSound.Play();
            ScoreManager.instance.AddPoint();

            transform.position = respawnPositions[i];
            i++;
        }
    }
    
}
