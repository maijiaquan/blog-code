#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
  
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform Material material;
uniform Light light;

// void main()
// {
//     // ambient
//     vec3 ambient = light.ambient * material.ambient;
  	
//     // diffuse 
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(light.position - FragPos);
//     float diff = max(dot(norm, lightDir), 0.0);
//     vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
//     // specular
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 reflectDir = reflect(-lightDir, norm);  
//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     vec3 specular = light.specular * (spec * material.specular);  
        
//     vec3 result = ambient + diffuse + specular;
//     FragColor = vec4(result, 1.0);
// } 


void main()
{    
    // 1.ambient
    vec3 ambientColor = lightColor * material.ambient;

    // 2.diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff =  max(dot(norm, lightDir), 0.0);
    vec3 diffuseColor = lightColor * diff * material.diffuse;

    // 3.specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specularColor = lightColor * spec * material.specular;  

    vec3 result = ambientColor + diffuseColor + specularColor;
    FragColor = vec4(result, 1.0);
}





// void main()
// {    
//     // 1.ambient
//     vec3 ambient = lightColor;
//     vec3 ambientColor = ambient * material.ambient;

//     // 2.diffuse 
//     vec3 norm = normalize(Normal);
//     vec3 lightDir = normalize(lightPos - FragPos);
//     float diffuse = max(dot(norm, lightDir), 0.0);
//     vec3 diffuseColor = lightColor * diffuse * material.diffuse;

//     // 3.specular
//     vec3 viewDir = normalize(viewPos - FragPos);
//     vec3 reflectDir = reflect(-lightDir, norm);  
//     float specular = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
//     vec3 specularColor = lightColor * specular * material.specular;  

//     vec3 result = ambientColor + diffuseColor + specularColor;
//     FragColor = vec4(result, 1.0);
// }