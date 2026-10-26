using Microsoft.AspNetCore.Identity;
using ResultsAuthenticate;

namespace ASPA0010_1.Data
{
    public static class DbInitializer
    {
        public static async Task InitializeAsync(IServiceProvider serviceProvider)
        {
            var context = serviceProvider.GetRequiredService<AuthDbContext>();
            var roleManager = serviceProvider.GetRequiredService<RoleManager<IdentityRole>>();
            var userManager = serviceProvider.GetRequiredService<UserManager<IdentityUser>>();

            context.Database.EnsureCreated();

            if (!await roleManager.RoleExistsAsync("READER"))
                await roleManager.CreateAsync(new IdentityRole("READER"));

            if (!await roleManager.RoleExistsAsync("WRITER"))
                await roleManager.CreateAsync(new IdentityRole("WRITER"));

            if (await userManager.FindByNameAsync("reader") == null)
            {
                var reader = new IdentityUser { UserName = "reader" };
                await userManager.CreateAsync(reader, "123");
                await userManager.AddToRoleAsync(reader, "READER");
            }

            if (await userManager.FindByNameAsync("writer") == null)
            {
                var writer = new IdentityUser { UserName = "writer" };
                await userManager.CreateAsync(writer, "123");
                await userManager.AddToRoleAsync(writer, "WRITER");
            }
        }
    }
}