using System.Collections;
using UnityEngine;
using UnityEngine.SceneManagement;

public class LoadLevel : MonoBehaviour
{
    public Animator transition; 
    void Update()
    {
        if(Input.GetKeyDown("space")){
            LoadNextLevel();
        }
    }
    
    public void LoadNextLevel(){
        StartCoroutine(LevelLoad());
    }
    //coroutine to load next scene
    //coroutines are one way to time events in code
    //they have a very specific syntax
    IEnumerator LevelLoad(){
        transition.SetTrigger("Start");

        yield return new WaitForSeconds(1);

        SceneManager.LoadScene("Game Scene");
    }
}
