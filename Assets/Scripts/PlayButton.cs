using System.Collections;
using UnityEngine;
using UnityEngine.SceneManagement;

public class PlayButton : MonoBehaviour
{
    public void PlayTheGame()
    {
        SceneManager.LoadScene("Game Scene");
    } 
}
